#pragma once

#include "helpers/clamp.h"
#include "audio/EnvelopDrumAmp.h"
#include "audio/lookupTable.h"
#include "audio/effects/applyBandpass.h"

#include <cstdint>

class DrumClapEngine {
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

        return applyBandpass(x, f0, Q, sampleRate, bp_x1, bp_x2, bp_y1, bp_y2);
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

    DrumClapEngine(int sampleRate, LookupTable& lookupTable)
        : sampleRate(sampleRate)
        , lookupTable(lookupTable)
    {
        envelopAmp.morph(0.0f);
    }

    void setBurstSpacing(float value) { burstSpacing = CLAMP(value, 0.0f, 1.0f); }
    void setDecay(float value) { decay = CLAMP(value, 0.0f, 1.0f); }
    void setBurstCount(int value) { burstCount = CLAMP(value, 1, 10); }
    void setNoiseColor(float value) { noiseColor = CLAMP(value, 0.0f, 1.0f); }
    void setPunch(float value) { punch = CLAMP(value, -1.0f, 1.0f); }
    void setFilter(float value) { filter = CLAMP(value, 0.0f, 1.0f); }
    void setResonance(float value) { resonance = CLAMP(value, 0.0f, 1.0f); }

    void noteOn(uint8_t note)
    {
        float spacing = burstSpacing * 0.03f + 0.01f;
        float decayTime = decay * 0.3f + 0.02f;
        float totalClapTime = (burstCount - 1) * spacing + decayTime * 3.0f;
        int clapTotalSamples = sampleRate * totalClapTime;
        envelopAmp.reset(clapTotalSamples);
        clapActive = true;
        burstTimer = 0.f;
        burstIndex = 0;
        clapEnv = 1.f;
        pink = 0.f;
    }

protected:
    float burstTimer = 0.f;
    int burstIndex = 0;
    float clapEnv = 0.0f;
    float pink = 0.0f;
    bool clapActive = false;

public:
    float sample()
    {
        float out = 0.0f;
        if (clapActive) {
            float envAmp = envelopAmp.next();
            float spacing = burstSpacing * 0.03f + 0.01f;
            float decayTime = decay * 0.3f + 0.02f;

            if (burstIndex < int(burstCount)) {
                burstTimer += 1.f / sampleRate;
                if (burstTimer >= spacing) {
                    burstTimer -= spacing;
                    burstIndex++;
                    clapEnv = 1.f;
                }
            }

            if (clapEnv > 0.f) {
                // Pink noise
                float white = lookupTable.getNoise() * 2.f - 1.f;
                pink = 0.98f * pink + 0.02f * white;
                float noise = pink * (1.f - noiseColor) + white * noiseColor;

                float burst = noise * clapEnv;
                out += burst;

                clapEnv *= expf(-1.f / (sampleRate * decayTime));
            } else if (burstIndex >= int(burstCount)) {
                clapActive = false;
            }

            if (punch < 0.0f) {
                // out = CLAMP(out + out * -punch * 8, -1.0f, 1.0f);
                if (burstIndex < int(burstCount * 0.5f)) {
                    out *= 1.f + -punch * 2.f;
                }
            } else if (punch > 0.0f) {
                float t = burstTimer / spacing;
                if (t < 0.02f) {
                    out *= 1.f + punch * 2.f;
                }
            }

            out = applyBandpassFx(out);

            out *= envAmp;
        }

        return out;
    }
};