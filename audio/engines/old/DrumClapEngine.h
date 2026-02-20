/** Description:
This document defines the blueprint for a specialized digital sound generator called the `DrumClapEngine`. Its specific function is to synthesize the characteristic sound of a hand clap, often used in electronic music or drum machines. It operates as a component within a larger audio processing system.

**How the Sound is Created:**

The engine generates the clap sound not through traditional musical tones, but through carefully shaped static noise.

1.  **Colored Noise Generation:** It prepares a large bank of noise data. The user can adjust the `noiseColor` parameter to blend between sharp, aggressive white noise and softer, bass-heavy pink noise, which is stored in an internal lookup table for efficiency.
2.  **Clap Structure (Bursts):** A natural clap is a rapid sequence of events. The engine simulates this by triggering multiple short noise "bursts." Users control how many bursts occur (`burstCount`) and the time between them (`burstSpacing`).
3.  **Filtering and Shaping:** The raw noise bursts are then passed through a digital **Bandpass Filter**. This filter is crucial, as it removes very high and very low frequencies, isolating the specific midrange frequencies (around 1 kHz to 4 kHz) that give a clap its signature tone. The intensity of this filtering can be adjusted using `filter` and `resonance` controls.
4.  **Dynamics:** An overall volume curve (envelope) dictates the main fade-out (`decay`) of the entire sound. An adjustable `punch` parameter allows the user to emphasize or soften the initial attack of the clap.

The engine includes robust controls for setting parameters (like `decay` or `noiseColor`) and functions to start the sound (`noteOn`) and generate the next tiny segment of audio (`sample()`). It also allows its internal state to be saved and loaded (`serialize`/`hydrate`) for seamless use in digital audio applications.

sha: 067036dfc9a8d719d0ea6bd587bcd1d51322c555a1aabf23e0b1a1ae47571be5 
*/
#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/engines/Engine.h"
#include "audio/lookupTable.h"
#include "helpers/clamp.h"
#include "helpers/math.h"
#ifdef USE_LUT_AND_FAST_MATH
#include "audio/effects/applyBandpassFast.h"
#else
#include "audio/effects/applyBandpass.h"
#endif

#include <cstdint>

class DrumClapEngine : public Engine {
protected:
    int sampleRate;
    LookupTable& lookupTable;

    float bp_x1 = 0.f, bp_x2 = 0.f;
    float bp_y1 = 0.f, bp_y2 = 0.f;
    float applyBandpassFx(float x)
    {
        // Biquad bandpass filter (cookbook formula)
        float f0 = 1000.f + filter * 3000.f; // 1kHz to 4kHz
        float Q = 1.0f + resonance * 3.0f; // Q: 1 to 4

#ifdef USE_LUT_AND_FAST_MATH
        return applyBandpassFast(x, f0, Q, sampleRate, bp_x1, bp_x2, bp_y1, bp_y2);
#else
        return applyBandpass(x, f0, Q, sampleRate, bp_x1, bp_x2, bp_y1, bp_y2);
#endif
    }

    static const int noiseSize = 4096;
    float noise[noiseSize] = { 0.f };
    // TODO
    // TODO we might want to debounce to avoid to regenerate to often when we turn knobs...
    // TODO
    void generateNoiseLut()
    {
        for (int i = 0; i < noiseSize; i++) {
            float white = lookupTable.getNoise() * 2.f - 1.f;
            pink = 0.98f * pink + 0.02f * white;
            noise[i] = pink * (1.f - noiseColor) + white * noiseColor;
        }
    }
    int noiseIndex = 0;
    float getNoise()
    {
        noiseIndex = (noiseIndex + 1);
        if (noiseIndex >= noiseSize) noiseIndex = 0;
        return noise[noiseIndex];
    }

public:
    EnvelopDrumAmp envelopAmp;

    float burstSpacing = 0.5f; // 0.0 to 1.0
    float decay = 1.0f; // 0.0 to 1.0
    int8_t burstCount = 4; // 1 to 10
    float noiseColor = 0.5f; // 0.0 to 1.0
    float punch = 0.0f; // -1.0 to 1.0
    float filter = 0.0f; // 0.0 to 1.0
    float resonance = 0.0f; // 0.0 to 1.0

    void hydrate(const std::vector<KeyValue>& values) override
    {
        for (auto& kv : values) {
            if (kv.key == "burstSpacing") burstSpacing = std::get<float>(kv.value);
            else if (kv.key == "decay") decay = std::get<float>(kv.value);
            else if (kv.key == "burstCount") burstCount = std::get<float>(kv.value);
            else if (kv.key == "noiseColor") noiseColor = std::get<float>(kv.value);
            else if (kv.key == "punch") punch = std::get<float>(kv.value);
            else if (kv.key == "filter") filter = std::get<float>(kv.value);
            else if (kv.key == "resonance") resonance = std::get<float>(kv.value);
            else if (kv.key == "envelopAmp") envelopAmp.morph(std::get<float>(kv.value));
        }
    }
    std::vector<KeyValue> serialize() const override { return {
        { "burstSpacing", burstSpacing },
        { "decay", decay },
        { "burstCount", (float)burstCount },
        { "noiseColor", noiseColor },
        { "punch", punch },
        { "filter", filter },
        { "resonance", resonance },
        { "envelopAmp", envelopAmp.getMorph() },
    }; }

    DrumClapEngine(int sampleRate, LookupTable& lookupTable)
        : Engine(Engine::Type::Drum, "Clap", "Clap")
        , sampleRate(sampleRate)
        , lookupTable(lookupTable)
    {
        envelopAmp.morph(0.0f);
        generateNoiseLut();
        burstTimerInc = 1.f / sampleRate;
    }

    void setBurstSpacing(float value)
    {
        burstSpacing = CLAMP(value, 0.0f, 1.0f);
        updateTotalSamples();
    }
    void setDecay(float value)
    {
        decay = CLAMP(value, 0.0f, 1.0f);
        updateTotalSamples();
    }
    void setBurstCount(int value)
    {
        burstCount = CLAMP(value, 1, 10);
        updateTotalSamples();
    }
    void setNoiseColor(float value)
    {
        noiseColor = CLAMP(value, 0.0f, 1.0f);
        generateNoiseLut();
    }
    void setPunch(float value) { punch = CLAMP(value, -1.0f, 1.0f); }
    void setFilter(float value) { filter = CLAMP(value, 0.0f, 1.0f); }
    void setResonance(float value) { resonance = CLAMP(value, 0.0f, 1.0f); }

    void noteOn(uint8_t note) override
    {
        if (totalSamples == 0) updateTotalSamples();
        envelopAmp.reset(totalSamples);
        clapActive = true;
        burstTimer = 0.f;
        burstIndex = 0;
        clapEnv = 1.f;
        pink = 0.f;
    }

protected:
    void updateTotalSamples()
    {
        spacing = burstSpacing * 0.03f + 0.01f;
        decayTime = decay * 0.3f + 0.02f;
        float totalClapTime = (burstCount - 1) * spacing + decayTime * 3.0f;
        totalSamples = sampleRate * totalClapTime;
    }

    int totalSamples = 0;
    float burstTimer = 0.f;
    int burstIndex = 0;
    float clapEnv = 0.0f;
    float pink = 0.0f;
    bool clapActive = false;
    float spacing = 0.0f;
    float decayTime = 0.0f;
    float burstTimerInc = 0.0f;

public:
    float sample() override
    {
        float out = 0.0f;
        if (clapActive) {
            float envAmp = envelopAmp.next();

            if (burstIndex < int(burstCount)) {
                burstTimer += burstTimerInc;
                if (burstTimer >= spacing) {
                    burstTimer -= spacing;
                    burstIndex++;
                    clapEnv = 1.f;
                }
            }

            if (clapEnv > 0.f) {
                float burst = getNoise() * clapEnv;
                out += burst;
                clapEnv *= superFastExpf2(-1.f / (sampleRate * decayTime));
            } else if (burstIndex >= int(burstCount)) {
                clapActive = false;
            }

            // TODO
            // TODO improve punch
            // TODO
            if (punch < 0.0f) {
                if (burstIndex < int(burstCount * 0.5f)) {
                    out *= 1.f + -punch;
                }
            } else if (punch > 0.0f) {
                float t = burstTimer / spacing;
                if (t < 0.02f) {
                    out *= 1.f + punch * 1.5f;
                }
            }

            out = applyBandpassFx(out);

            out *= envAmp;
        }

        return out;
    }
};