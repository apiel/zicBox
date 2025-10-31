#pragma once

#include "helpers/clamp.h"
#include "audio/lookupTable.h"

#include <cstdint>
#include <algorithm>

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

    float lerp(float a, float b, float t) { return a + (b - a) * t; }

    float tableSin(float phase)
    {
        int idx = (int)(phase * lookupTable.size) % lookupTable.size;
        if (idx < 0) idx += lookupTable.size;
        return lookupTable.sine[idx];
    }

public:
    float decay = 0.5f;        // 0–1
    float toneFreq = 180.0f;   // 40–4000 Hz
    float mix = 0.5f;          // noise/tone mix
    float filter = 0.5f;       // 0–1 brightness
    float resonance = 0.5f;    // 0–1 filter Q
    float type = 0.0f;         // 0=snare → 1=hi-hat morph
    float toneTimbre = 0.5f;   // 0–1 harmonic richness
    float toneFM = 0.0f;       // 0–1 FM modulation depth

    DrumSnareHatEngine(int sampleRate, LookupTable& lookupTable)
        : sampleRate(sampleRate), lookupTable(lookupTable)
    {}

    void setDecay(float v) { decay = CLAMP(v, 0.f, 1.f); }
    void setToneFreq(float v) { toneFreq = CLAMP(v, 40.f, 4000.f); }
    void setMix(float v) { mix = CLAMP(v, 0.f, 1.f); }
    void setFilter(float v) { filter = CLAMP(v, 0.f, 1.f); }
    void setResonance(float v) { resonance = CLAMP(v, 0.f, 1.f); }
    void setType(float v) { type = CLAMP(v, 0.f, 1.f); }
    void setToneTimbre(float v) { toneTimbre = CLAMP(v, 0.f, 1.f); }
    void setToneFM(float v) { toneFM = CLAMP(v, 0.f, 1.f); }

    void noteOn(uint8_t)
    {
        envValue = 1.0f;

        float baseDecaySnare = 0.15f;
        float baseDecayHat   = 0.07f;
        float decayExtraSnare = 0.25f;
        float decayExtraHat   = 0.10f;
        float baseDecay = lerp(baseDecaySnare, baseDecayHat, type);
        float extra = lerp(decayExtraSnare, decayExtraHat, type);
        float decayTime = baseDecay + decay * extra;
        envDecayCoeff = expf(-1.f / (sampleRate * decayTime));

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

        float envAmp = envValue;
        envValue *= envDecayCoeff;

        // --- Noise Layer ---
        float white = lookupTable.getNoise() * 2.f - 1.f;
        float fSnare = 2000.f + filter * 4000.f;
        float fHat   = 6000.f + filter * 8000.f;
        float f0 = lerp(fSnare, fHat, type);
        float Q = 1.0f + resonance * 4.0f;
        float noise = applyBandpass(white, f0, Q);

        // --- Tone Layer with FM ---
        float fmFreq = currentToneFreq * 2.0f;
        float fmIndex = toneFM * 50.0f;
        float modulator = tableSin(fmFreq * phase) * fmIndex;
        float tone = tableSin(phase * currentToneFreq + modulator);

        // Harmonics
        if(toneTimbre > 0.01f) {
            tone += toneTimbre * (
                0.3f * tableSin(phase * currentToneFreq * 2.f) +
                0.2f * tableSin(phase * currentToneFreq * 3.3f)
            );
        }

        // Metallic partials fade in with type
        if(type > 0.05f) {
            float metalMix = type;
            tone += metalMix * (
                0.15f * tableSin(phase * currentToneFreq * 2.5f) +
                0.1f  * tableSin(phase * currentToneFreq * 3.7f)
            );
        }

        phase += phaseInc;
        if(phase >= 1.0f) phase -= 1.0f;

        float out = (noise * mix) + (tone * (1.0f - mix));
        return out * envAmp;
    }
};
