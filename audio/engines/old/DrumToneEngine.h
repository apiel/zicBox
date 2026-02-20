/** Description:
This code defines a specialized digital sound module called the `DrumToneEngine`. Its primary purpose is to generate realistic or synthetic drum sounds, such as kick drums, by combining several audio components and precisely shaping them over time.

### How the Engine Works

The engine operates by calculating the sound one tiny step (sample) at a time, based on a "Note On" trigger. It constructs the final sound through five key stages:

1.  **Waveform Generation:** It starts with a basic, raw tone (the waveform).
2.  **Frequency Modulation:** A rapid, adjustable change in pitch occurs right at the start of the drum hit, which is crucial for achieving a deep, powerful percussive sound.
3.  **Transient Addition:** An optional, very short "click" or attack sound can be layered on top to mimic the initial impact of a beater hitting a drumhead.
4.  **Dynamic Filtering:** The tone is passed through a frequency filter (specifically a Low-Pass Filter) which acts like a dimmer switch for high-frequency content. The setting of this filter changes dynamically as the drum sound progresses, greatly sculpting the final timbre.
5.  **Amplitude Envelope:** Finally, an amplitude envelope controls the overall volume, defining how quickly the sound fades away (its duration).

### Key Adjustable Parameters

Users can customize the drum sound using various controls:

*   **Duration and Pitch:** Sets the overall length of the decay and the fundamental tone of the drum.
*   **Filter Cutoff:** Controls the maximum brightness allowed by the tone filter.
*   **Morphing Parameters:** Allows seamless adjustments to the shapes of the frequency envelope, the amplitude envelope, and the characteristics of the transient click, providing extensive sonic variety.

In essence, the `DrumToneEngine` acts as a customizable recipe, mixing multiple simple acoustic processes (tone, volume, pitch change, filtering) to synthesize a complex, professional-grade drum sound.

sha: fc175d19745f5f6b87c43cc790a8dfa1aac9f7b46982d2568908be7aa02daf72 
*/
#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/KickEnvTableGenerator.h"
#include "audio/KickTransientTableGenerator.h"
#include "audio/WavetableGenerator2.h"
#include "audio/engines/Engine.h"
#include "audio/filterArray.h"
#include "audio/lookupTable.h"

class DrumToneEngine : public Engine {
protected:
    int sampleRate;

    EffectFilterArray<3> filter;
    float resonance = 0.0f; // 0.0 to 1.0

    float freq = 1.0f;

    // TODO
    // TODO use only LPF
    // TODO
    float applyFilter(float out, float envAmp)
    {
        // if (filterCutoff < 0) { // HPF
        //     float amount = -filterCutoff;
        //     filter.setCutoff(0.85 * amount * envAmp + 0.1);
        //     filter.setSampleData(out, 0);
        //     filter.setSampleData(filter.hp[0], 1);
        //     filter.setSampleData(filter.hp[1], 2);
        //     out = filter.lp[0] * (1.0f - amount) + filter.hp[2] * amount; // Smooth transition between LPF and HPF
        // } else { // LPF
        //     filter.setCutoff(0.85 * filterCutoff * envAmp + 0.1);
        //     filter.setSampleData(out, 0);
        //     filter.setSampleData(filter.lp[0], 1);
        //     filter.setSampleData(filter.lp[1], 2);
        //     out = filter.lp[2];
        // }
        filter.setCutoff(0.85 * filterCutoff * envAmp + 0.1);
        filter.setSampleData(out, 0);
        filter.setSampleData(filter.lp[0], 1);
        filter.setSampleData(filter.lp[1], 2);
        out = filter.lp[2];
        return out;
    }

public:
    WavetableGenerator waveform;
    EnvelopDrumAmp envelopAmp;
    KickEnvTableGenerator envelopFreq;
    KickTransientTableGenerator transient;

    int duration = 1000; // 50 to 3000
    int8_t pitch = -8; // -36 to 36

    // Filter
    float filterCutoff = 0.0f; // -1.0 to 1.0

    void hydrate(const std::vector<KeyValue>& values) override
    {
        for (auto& kv : values) {
            if (kv.key == "duration") setDuration(std::get<float>(kv.value));
            else if (kv.key == "pitch") setPitch(std::get<float>(kv.value));
            else if (kv.key == "filterCutoff") setFilterCutoff(std::get<float>(kv.value));
            else if (kv.key == "waveformType") waveform.setType(std::get<std::string>(kv.value));
            else if (kv.key == "waveformMorph") waveform.setMorph(std::get<float>(kv.value));
            else if (kv.key == "envelopAmp") envelopAmp.morph(std::get<float>(kv.value));
            else if (kv.key == "envelopFreq") envelopFreq.setMorph(std::get<float>(kv.value));
            else if (kv.key == "transient") transient.setMorph(std::get<float>(kv.value));
        }
    }
    std::vector<KeyValue> serialize() const override { return {
        { "duration", (float)duration },
        { "pitch", (float)pitch },
        { "filterCutoff", filterCutoff },
        { "waveformType", waveform.getTypeName() },
        { "waveformMorph", waveform.getMorph() },
        { "envelopAmp", envelopAmp.getMorph() },
        { "envelopFreq", envelopFreq.getMorph() },
        { "transient", transient.getMorph() },
    }; }

    DrumToneEngine(int sampleRate, LookupTable* lookupTable)
        : Engine(Engine::Type::Drum, "Tone", "Tone")
        , sampleRate(sampleRate)
        , waveform(lookupTable, sampleRate)
    {
        waveform.setType(WavetableGenerator::Type::Saw);
        envelopAmp.morph(0.2f);
    }

    void setDuration(int value)
    {
        duration = CLAMP(value, 50, 3000);
        updateTotalSamples();
    }
    void setPitch(int value) { pitch = CLAMP(value, -36, 36); }
    // void setFilterCutoff(float value) { filterCutoff = CLAMP(value, -1.0f, 1.0f); }
    void setFilterCutoff(float value) { filterCutoff = CLAMP(value, 0.0f, 1.0f); }

    void setResonance(float value)
    {
        resonance = CLAMP(value, 0.0f, 1.0f);
        filter.setResonance(0.95 * (1.0 - std::pow(1.0 - resonance, 2)));
    }

    float getResonance() { return resonance; }

    // Higher base note is, lower pitch will be
    //
    // Usualy our base note is 60
    // but since we want a kick sound we want bass and we remove one octave (12 semitones)
    // So we send note 60, we will play note 48...
    const uint8_t baseNote = 60 + 12;

    void noteOn(uint8_t note) override
    {
        freq = pow(2, ((note - baseNote + pitch) / 12.0));
        envelopAmp.reset(totalSamples);
        sampleCounter = 0;
        sampleIndex = 0.0f;
        if (totalSamples == 0) {
            updateTotalSamples();
        }
    }

protected:
    void updateTotalSamples() { totalSamples = static_cast<int>(sampleRate * (duration / 1000.0f)); }

    int totalSamples = 0;
    int sampleCounter = 0;
    float sampleIndex = 0.0f;

public:
    float sample() override
    {
        if (sampleCounter < totalSamples) {
            float t = float(sampleCounter) / totalSamples;
            sampleCounter++;

            float envAmp = envelopAmp.next();
            float envFreq = envelopFreq.next(t);

            float modulatedFreq = freq + envFreq;
            float out = waveform.sample(&sampleIndex, modulatedFreq) * 0.75f;

            if (t < 0.01f && transient.getMorph() > 0.0f) {
                out = out + transient.next(t);
            }

            out = applyFilter(out, envAmp);

            return out * envAmp;
        }
        return 0.0f;
    }
};