/** Description:
This document describes a C++ component called `LoopedEngine`. This component is a specialized audio processing module designed to play back digital audio recordings, or "samples," with advanced control over repetition and sustain.

**Core Purpose**

The `LoopedEngine` manages sample playback where a specific section of the audio file can be looped indefinitely while a musical note is held down. It inherits basic features from a standard `SampleEngine` but adds sophisticated looping logic essential for instruments where a sound needs to sustain (like a synthesizer or piano pad).

**Key Functionality**

1.  **Pitch Adjustment:** When a musical note is played, the engine automatically calculates the necessary playback speed (step increment) to ensure the sample plays back at the correct pitch, relative to a configured base note.
2.  **Playback Control:** It allows defining the overall playback segment using controls for `Start` and `End` points of the sample.
3.  **Loop Definition:** Within the main segment, users define a precise `Loop Position` (where the loop starts) and `Loop Length`.
4.  **Sustain and Release:**
    *   When a key is pressed (`Note On`), playback begins. If the playback index reaches the loop end, it instantly jumps back to the loop start, creating an endless sustain.
    *   When the key is released (`Note Off`), the loop does not stop immediately. Instead, a special setting called "Loop Release" dictates how many final times the loop must complete before the sample is allowed to continue past the loop region toward the overall `End` point and fade out.

This engine handles the continuous calculation of the current position in the audio sample, ensuring seamless transitions and pitch accuracy throughout sustained and looped playback.

sha: 034d17d3e8172ccf7b9da0e811e90992e36faf101fbace65a06bfce879815868
*/
#pragma once

#include "plugins/audio/MultiSampleEngine/SampleEngine.h"

class LoopedEngine : public SampleEngine {
protected:
    uint64_t indexStart = 0;
    uint64_t indexEnd = 0;
    uint64_t loopStart = 0;
    uint64_t loopEnd = 0;
    uint16_t loopCountRelease = 0;
    int16_t nbOfLoopBeforeRelease = 0;
    float stepIncrement = 1.0;

    float velocity = 1.0;
    uint8_t sustainedNote = 0;

    uint8_t baseNote = 60;
    float getSampleStep(uint8_t note)
    {
        // https://gist.github.com/YuxiUx/ef84328d95b10d0fcbf537de77b936cd
        // pow(2, ((0) / 12.0)) = 1 for 0 semitone
        // pow(2, ((1) / 12.0)) = 1.059463094 for 1 semitone
        // pow(2, ((2) / 12.0)) = 1.122462048 for 2 semitone
        // ...

        // printf("getSampleStep: %d >> %d = %f\n", note, note - baseNote, pow(2, (note - baseNote) / 12.0));
        return pow(2, ((note - baseNote) / 12.0)) * stepMultiplier;
    }

    void opened() override
    {
        indexEnd = end.pct() * sampleBuffer.count;
        initValues();
    }

    Val& getValExtra()
    {
        return val(0.0f, "VAL_EXTRA", { "Loop Release", .min = 0.0, .max = 5000.0, .step = 50.0, .unit = "ms" }, [&](auto p) {
            p.val.setFloat(p.value);
            if (p.val.get() > 0) {
                uint64_t loopLength = sustainLength.pct() * sampleBuffer.count;
                float msLoopLength = loopLength / (float)props.sampleRate * 1000;
                loopCountRelease = msLoopLength > 0 ? p.val.get() / msLoopLength : 0;
                // printf(">>>> sustainRelease: %f loopLength: %ld msLoopLength: %f loopCountRelease: %d\n", p.val.get(), loopLength, msLoopLength, loopCountRelease);
            }
            p.val.props().unit = "ms(" + std::to_string(loopCountRelease) + ")";
        });
    }

public:
    Val& sustainPosition;
    Val& sustainLength;
    Val& valExtra;
    Val& start;
    Val& end;

    LoopedEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, SampleBuffer& sampleBuffer, float& index, float& stepMultiplier, std::string name, Val* browser, std::function<Val&()> getValExtraEngine = nullptr)
        : SampleEngine(props, config, sampleBuffer, index, stepMultiplier, name)
        // , browser(browser)
        , sustainPosition(val(0.0f, "LOOP_POSITION", { "Loop position", .step = 0.1f, .floatingPoint = 1, .unit = "%" }, [&](auto p) {
            if (p.value < start.get()) {
                p.value = start.get();
            }
            if (p.value + sustainLength.get() > end.get()) {
                return;
            }
            sustainPosition.setFloat(p.value);
            if (sustainLength.get() == 0.0f && p.val.get() > start.get()) {
                sustainLength.set(5);
            }
            loopStart = p.val.pct() * sampleBuffer.count;
            logTrace("- LOOP_POSITION: %d", loopStart);
            sustainLength.set(sustainLength.get());
            needCopyValues = true;
        }))
        , sustainLength(val(0.0f, "LOOP_LENGTH", { "Loop length", .step = 0.1f, .floatingPoint = 1, .unit = "%" }, [&](auto p) {
            if (p.value + sustainPosition.get() > end.get()) {
                return;
            }
            sustainLength.setFloat(p.value);
            loopEnd = p.val.pct() * sampleBuffer.count + sustainPosition.pct() * sampleBuffer.count;
            logTrace("- LOOP_LENGTH: %d", loopEnd);
            valExtra.set(valExtra.get());
            if (sustainLength.get() == 0) {
                sustainedNote = 0;
            }
        }))
        , valExtra(getValExtraEngine == nullptr ? this->getValExtra() : getValExtraEngine())
        , start(val(0.0f, "START", { "Start", .step = 0.1f, .floatingPoint = 1, .unit = "%" }, [&](auto p) {
            if (p.value < end.get()) {
                bool sustainEq = p.val.get() == sustainPosition.get();
                p.val.setFloat(p.value);
                indexStart = p.val.pct() * sampleBuffer.count;
                logTrace("- START: %d", indexStart);
                if (p.val.get() > sustainPosition.get() || sustainEq) {
                    sustainPosition.set(p.val.get());
                    needCopyValues = true;
                }
            }
        }))
        , end(val(100.0f, "END", { "End", .step = 0.1f, .floatingPoint = 1, .unit = "%" }, [&](auto p) {
            if (p.value > start.get()) {
                p.val.setFloat(p.value);
                indexEnd = p.val.pct() * sampleBuffer.count;
                logTrace("- END: %d", indexEnd);
                if (p.val.get() < sustainPosition.get() + sustainLength.get()) {
                    sustainPosition.set(p.val.get() - sustainLength.get());
                    needCopyValues = true;
                }
            }
        }))
    {
        mapping.insert(mapping.begin(), browser);
    }

    virtual float getSample(float stepIncrement)
    {
        return sampleBuffer.data[(int)index];
    }

    void sample(float* buf) override
    {
        float out = 0.0f;
        if (index < indexEnd) {
            out = getSample(stepIncrement) * velocity;
            index += stepIncrement;
            postIncrement();
        } else if (index != sampleBuffer.count) {
            index = sampleBuffer.count;
        }
        buf[track] = out;
        postProcess(buf);
    }

    virtual void postIncrement()
    {
        if (index >= loopEnd && (sustainedNote || nbOfLoopBeforeRelease > 0)) {
            index = loopStart;
            nbOfLoopBeforeRelease--;
        }
    }

    virtual void postProcess(float* buf) { }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        index = indexStart;
        indexEnd = end.pct() * sampleBuffer.count;
        stepIncrement = getSampleStep(note);
        velocity = _velocity;
        if (sustainLength.get() > 0.0f) {
            sustainedNote = note;
        }
        engineNoteOn(note, _velocity);
    }

    virtual void engineNoteOn(uint8_t note, float _velocity) { }

    virtual uint16_t getLoopCountRelease() { return loopCountRelease; }

    void noteOff(uint8_t note, float velocity, void* userdata = NULL) override
    {
        if (note == sustainedNote) {
            nbOfLoopBeforeRelease = getLoopCountRelease();
            sustainedNote = 0;
        }
        engineNoteOff(note, velocity);
    }

    virtual void engineNoteOff(uint8_t note, float _velocity) { }
};
