#pragma once

#include "helpers/clamp.h"
#include "audio/lookupTable.h"
#include <cmath>
#include <cstdint>
#include <algorithm>

class DrumSnareHatEngine {
protected:
    int sampleRate;
    LookupTable& lookupTable;

    // Filter state for noise bandpass
    float bp_x1 = 0.f, bp_x2 = 0.f;
    float bp_y1 = 0.f, bp_y2 = 0.f;

    float applyBandpass(float x, float center, float Q)
    {
        float omega = 2.f * M_PI * center / sampleRate;
        float alpha = sinf(omega) / (2.f * Q);

        float b0 = alpha;
        float b1 = 0.f;
        float b2 = -alpha;
        float a0 = 1.f + alpha;
        float a1 = -2.f * cosf(omega);
        float a2 = 1.f - alpha;

        float y = (b0 / a0) * x + (b1 / a0) * bp_x1 + (b2 / a0) * bp_x2
                - (a1 / a0) * bp_y1 - (a2 / a0) * bp_y2;

        bp_x2 = bp_x1;
        bp_x1 = x;
        bp_y2 = bp_y1;
        bp_y1 = y;

        return y;
    }

public:
    // Parameters
    float decay = 0.5f;       // 0.0–1.0
    float toneFreq = 180.0f;  // 80–800 Hz typical
    float mix = 0.5f;     // 0.0–1.0
    float filter = 0.5f;      // 0.0–1.0 brightness
    float resonance = 0.5f;   // 0.0–1.0
    float type = 0.0f;        // 0=snare, 1=hihat

    DrumSnareHatEngine(int sampleRate, LookupTable& lookupTable)
        : sampleRate(sampleRate), lookupTable(lookupTable)
    {}

    void setDecay(float value) { decay = CLAMP(value, 0.0f, 1.0f); }
    void setToneFreq(float value) { toneFreq = CLAMP(value, 40.f, 4040.f); }
    void setMix(float value) { mix = CLAMP(value, 0.0f, 1.0f); }
    void setFilter(float value) { filter = CLAMP(value, 0.0f, 1.0f); }
    void setResonance(float value) { resonance = CLAMP(value, 0.0f, 1.0f); }
    void setType(float value) { type = CLAMP(value, 0.0f, 1.0f); }

    void noteOn(uint8_t)
    {
        envValue = 1.0f;
        float baseDecay = (type < 0.5f) ? 0.15f : 0.07f; // snare vs hat
        float extra = decay * ((type < 0.5f) ? 0.25f : 0.1f);
        float decayTime = baseDecay + extra;
        envDecayCoeff = expf(-1.f / (sampleRate * decayTime));

        phase = 0.0f;
        phaseInc = toneFreq / sampleRate;
    }

protected:
    float envValue = 0.0f;
    float envDecayCoeff = 0.0f;
    float phase = 0.0f;
    float phaseInc = 0.0f;

public:
    float sample()
    {
        if (envValue < 0.0001f) return 0.f;

        // --- Envelope ---
        float envAmp = envValue;
        envValue *= envDecayCoeff;

        // --- Noise layer ---
        float white = lookupTable.getNoise() * 2.f - 1.f;
        float f0 = (type < 0.5f)
            ? (2000.f + filter * 4000.f)  // snare
            : (6000.f + filter * 8000.f); // hat
        float Q = 1.0f + resonance * 4.0f;
        float noise = applyBandpass(white, f0, Q);

        // --- Tone layer ---
        float tone = sinf(2.f * M_PI * phase);
        phase += phaseInc;
        if (phase >= 1.0f) phase -= 1.0f;

        // --- Combine layers ---
        float out = (noise * mix) + (tone * (1.0f - mix));

        // Optional hat metallic partials
        if (type >= 0.5f) {
            out += 0.3f * sinf(2.f * M_PI * phase * 2.0f);
            out += 0.2f * sinf(2.f * M_PI * phase * 3.3f);
        }

        return out * envAmp;
    }
};
