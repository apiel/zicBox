#pragma once

#include "audio/lookupTable.h"
#include "helpers/clamp.h"
#include "helpers/math.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

class DrumSnareHatEngine {
protected:
    int sampleRate;
    LookupTable& lookupTable;

    // Bandpass filter state
    float bp_x1 = 0.f, bp_x2 = 0.f;
    float bp_y1 = 0.f, bp_y2 = 0.f;

    float applyBandpass(float x, float center, float Q)
    {
        float omega = 2.f * M_PI * center / sampleRate;
        float alpha = fastSin(omega) / (2.f * Q);

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

    float lerp(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

public:
    // User parameters
    float decay = 0.5f; // 0–1
    float toneFreq = 180.0f; // 40–4000 Hz
    float mix = 0.5f; // 0–1 (noise/tone)
    float filter = 0.5f; // 0–1
    float resonance = 0.5f; // 0–1
    float type = 0.0f; // 0=snare → 1=hi-hat (morph)

    DrumSnareHatEngine(int sampleRate, LookupTable& lookupTable)
        : sampleRate(sampleRate)
        , lookupTable(lookupTable)
    {
    }

    void setDecay(float v) { decay = CLAMP(v, 0.f, 1.f); }
    void setToneFreq(float v) { toneFreq = CLAMP(v, 40.f, 4040.f); }
    void setMix(float v) { mix = CLAMP(v, 0.f, 1.f); }
    void setFilter(float v) { filter = CLAMP(v, 0.f, 1.f); }
    void setResonance(float v) { resonance = CLAMP(v, 0.f, 1.f); }
    void setType(float v) { type = CLAMP(v, 0.f, 1.f); }

    void noteOn(uint8_t)
    {
        envValue = 1.0f;

        // --- Smooth morph between snare (t=0) and hat (t=1) ---
        float baseDecaySnare = 0.15f;
        float baseDecayHat = 0.07f;
        float decayExtraSnare = 0.25f;
        float decayExtraHat = 0.10f;
        float baseDecay = lerp(baseDecaySnare, baseDecayHat, type);
        float extra = lerp(decayExtraSnare, decayExtraHat, type);
        float decayTime = baseDecay + decay * extra;

        envDecayCoeff = expf(-1.f / (sampleRate * decayTime));

        // tone morph: from deep (snare) to bright (hat)
        float toneFreqSnare = toneFreq;
        float toneFreqHat = std::min(2000.f, toneFreq * 4.f);
        currentToneFreq = lerp(toneFreqSnare, toneFreqHat, type);

        phase = 0.0f;
        phaseInc = currentToneFreq / sampleRate;
    }

protected:
    float envValue = 0.0f;
    float envDecayCoeff = 0.0f;
    float phase = 0.0f;
    float phaseInc = 0.0f;
    float currentToneFreq = 0.0f;

public:
    float sample()
    {
        if (envValue < 0.0001f)
            return 0.f;

        // --- Envelope ---
        float envAmp = envValue;
        envValue *= envDecayCoeff;

        // --- Noise ---
        float white = lookupTable.getNoise() * 2.f - 1.f;
        // Center frequency smoothly morphs from 2–6kHz → 6–14kHz
        float fSnare = 2000.f + filter * 4000.f;
        float fHat = 6000.f + filter * 8000.f;
        float f0 = lerp(fSnare, fHat, type);
        float Q = 1.0f + resonance * 4.0f;
        float noise = applyBandpass(white, f0, Q);

        // --- Tone ---
        float tone = sinf(2.f * M_PI * phase);
        phase += phaseInc;
        if (phase >= 1.0f)
            phase -= 1.0f;

        // --- Metallic partials fade in as type → 1.0 (hi-hat) ---
        if (type > 0.0f) {
            float metalMix = type; // 0 = no metal, 1 = full hat partials
            tone += metalMix * (0.3f * sinf(2.f * M_PI * phase * 2.0f) + 0.2f * sinf(2.f * M_PI * phase * 3.3f));
        }

        // --- Mix tone/noise ---
        float out = (noise * mix) + (tone * (1.0f - mix));

        return out * envAmp;
    }
};
