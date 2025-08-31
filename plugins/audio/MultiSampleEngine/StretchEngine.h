#pragma once

#include "plugins/audio/MultiSampleEngine/SampleEngine.h"
#include "plugins/audio/utils/MMfilter.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/val/valMMfilterCutoff.h"

#include <cmath>
#include <cstdlib>

class StretchEngine : public SampleEngine {
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

    MultiFx multiFx;
    MMfilter filter;

    Val& sustainPosition = val(0.0f, "LOOP_POSITION", { "Loop position", .step = 0.1f, .floatingPoint = 1, .unit = "%" }, [&](auto p) {
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
    });

    Val& sustainLength = val(0.0f, "LOOP_LENGTH", { "Loop length", .step = 0.1f, .floatingPoint = 1, .unit = "%" }, [&](auto p) {
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
    });

    Val& valExtra = val(0.0f, "LOOP_RELEASE", { "Loop Release", .min = 0.0, .max = 5000.0, .step = 50.0, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.get() > 0) {
            uint64_t loopLength = sustainLength.pct() * sampleBuffer.count;
            float msLoopLength = loopLength / (float)props.sampleRate * 1000;
            loopCountRelease = msLoopLength > 0 ? p.val.get() / msLoopLength : 0;
            // printf(">>>> sustainRelease: %f loopLength: %ld msLoopLength: %f loopCountRelease: %d\n", p.val.get(), loopLength, msLoopLength, loopCountRelease);
        }
        p.val.props().unit = "ms(" + std::to_string(loopCountRelease) + ")";
    });

    Val& start = val(0.0f, "START", { "Start", .step = 0.1f, .floatingPoint = 1, .unit = "%" }, [&](auto p) {
        if (p.value < end.get()) {
            bool sustainEq = p.val.get() == sustainPosition.get();
            p.val.setFloat(p.value);
            indexStart = p.val.pct() * sampleBuffer.count;
            logTrace("- START: %d", indexStart);
            if (p.val.get() > sustainPosition.get() || sustainEq) {
                sustainPosition.set(p.val.get());
            }
        }
    });

    Val& end = val(100.0f, "END", { "End", .step = 0.1f, .floatingPoint = 1, .unit = "%" }, [&](auto p) {
        if (p.value > start.get()) {
            p.val.setFloat(p.value);
            indexEnd = p.val.pct() * sampleBuffer.count;
            logTrace("- END: %d", indexEnd);
            if (p.val.get() < sustainPosition.get() + sustainLength.get()) {
                sustainPosition.set(p.val.get() - sustainLength.get());
            }
        }
    });

    Val& stretch = val(1.0f, "STRETCH", { "Stretch", .min = 1.0f, .max = 20.0f });
    Val& grainSize = val(100.0f, "GRAIN_SIZE", { "Grain Size", .min = 1.0f, .max = 1000.0f });

    Val& cutoff = val(0.0, "CUTOFF", { "LPF | HPF", VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        filter.setResonance(p.val.pct());
    });

    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 },
        [&](auto p) { multiFx.setFxType(p); });
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX amount", .unit = "%" });

public:
    StretchEngine(AudioPlugin::Props& props, AudioPlugin::Config& config,
        SampleBuffer& sampleBuffer, float& index, float& stepMultiplier)
        : SampleEngine(props, config, sampleBuffer, index, stepMultiplier, "Stretch")
        , multiFx(props.sampleRate, props.lookupTable)
    {
    }

    int stretchLoopCount = 0;
    float grainStart = 0.0f;
    void sample(float* buf) override
    {
        float out = 0.0f;
        if (index < indexEnd) {
            out = sampleBuffer.data[(int)index] * velocity;
            index += stepIncrement;

            if (index >= grainStart + grainSize.get()) {
                if (stretchLoopCount < stretch.get() - 1) {
                    index = grainStart;
                    stretchLoopCount++;
                } else {
                    grainStart = index;
                    stretchLoopCount = 0;
                }
            }

            if (index >= loopEnd && (sustainedNote || nbOfLoopBeforeRelease > 0)) {
                index = loopStart;
                nbOfLoopBeforeRelease--;
            }
            out = filter.process(out);
        } else if (index != sampleBuffer.count) {
            index = sampleBuffer.count;
        }

        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        index = indexStart;
        grainStart = indexStart;
        stretchLoopCount = 0;
        indexEnd = end.pct() * sampleBuffer.count;
        stepIncrement = getSampleStep(note);
        velocity = _velocity;
        if (sustainLength.get() > 0.0f) {
            sustainedNote = note;
        }
    }

    void noteOff(uint8_t note, float velocity, void* userdata = NULL) override
    {
        if (note == sustainedNote) {
            nbOfLoopBeforeRelease = loopCountRelease;
            sustainedNote = 0;
        }
    }
};
