#pragma once

#include "audio/lookupTable.h"
#include "helpers/clamp.h"
#ifdef USE_LUT_AND_FAST_MATH
#include "audio/effects/applyBandpassFast.h"
#else
#include "audio/effects/applyBandpass.h"
#endif

#include <algorithm>
#include <cstdint>

class DrumSnareHatEngine {
protected:
    int sampleRate;
    LookupTable& lookupTable;

    // Bandpass filter state
    float bp_x1 = 0.f, bp_x2 = 0.f;
    float bp_y1 = 0.f, bp_y2 = 0.f;

    float lerp(float a, float b, float t) { return a + (b - a) * t; }

public:
    // Parameters
    float decay = 0.5f; // 0–1
    float toneFreq = 180.0f; // 40–4000 Hz
    float mix = 0.5f; // noise/tone mix
    float filter = 0.5f; // 0–1 brightness
    float resonance = 0.5f; // 0–1 filter Q
    float type = 0.0f; // 0=snare → 1=hi-hat morph
    float toneTimbre = 0.0f; // 0–1 harmonic richness
    float toneFM = 0.0f; // 0–1 FM modulation depth

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
    void setToneTimbre(float v) { toneTimbre = CLAMP(v, 0.f, 1.f); }
    void setToneFM(float v) { toneFM = CLAMP(v, 0.f, 1.f); }

    void noteOn(uint8_t)
    {
        envValue = 1.0f;

        // Decay morph: snare → hat
        float baseDecaySnare = 0.08f;
        float baseDecayHat = 0.04f;
        float decayExtraSnare = 0.25f;
        float decayExtraHat = 0.10f;
        float baseDecay = lerp(baseDecaySnare, baseDecayHat, type);
        float extra = lerp(decayExtraSnare, decayExtraHat, type);
        float decayTime = baseDecay + decay * extra;
        envDecayCoeff = expf(-1.f / (sampleRate * decayTime));

        // Tone frequency morph
        float toneFreqSnare = toneFreq;
        float toneFreqHat = std::min(2000.f, toneFreq * 4.f);
        currentToneFreq = lerp(toneFreqSnare, toneFreqHat, type);

        // Precompute bandpass coefficients
        float centerFreq = lerp(2000.f + filter * 4000.f, 6000.f + filter * 8000.f, type);
        float Q = 1.f + resonance * 4.f;
        bpCoeffs = { centerFreq, Q }; // store for sample()

        // Initialize phases
        phase = 0.0f;
        phaseInc = currentToneFreq / sampleRate;
        fmPhase = 0.0f;
    }

protected:
    float envValue = 0.0f;
    float envDecayCoeff = 0.0f;
    float phase = 0.0f;
    float phaseInc = 0.0f;
    float fmPhase = 0.0f;
    float currentToneFreq = 0.0f;

    struct {
        float center;
        float Q;
    } bpCoeffs;

public:
    float sample()
    {
        if (envValue < 0.0001f)
            return 0.f;

        float envAmp = envValue;
        envValue *= envDecayCoeff;

        // --- Noise Layer ---
        float white = lookupTable.getNoise() * 2.f - 1.f;
#ifdef USE_LUT_AND_FAST_MATH
        float noise = applyBandpassFast(white, bpCoeffs.center, bpCoeffs.Q, sampleRate, bp_x1, bp_x2, bp_y1, bp_y2);
#else
        float noise = applyBandpass(white, bpCoeffs.center, bpCoeffs.Q, sampleRate, bp_x1, bp_x2, bp_y1, bp_y2);
#endif

        // --- FM Oscillator ---
        float fmFreq = currentToneFreq * 2.0f; // FM frequency
        fmPhase += fmFreq / sampleRate;
        if (fmPhase >= 1.0f)
            fmPhase -= 1.0f;
        float modulator = toneFM * 0.1f * lookupTable.getSin(fmPhase); // FM depth

        // --- Tone Layer ---
        float tone = lookupTable.getSin(phase + modulator);

        // Harmonics
        if (toneTimbre > 0.0f) {
            tone += toneTimbre * (0.3f * lookupTable.getSin(phase * 2.f) + 0.2f * lookupTable.getSin(phase * 3.3f));
        }

        // Metallic partials fade in with type
        if (type > 0.05f) {
            float metalMix = type;
            tone += metalMix * (0.15f * lookupTable.getSin(phase * 2.5f) + 0.1f * lookupTable.getSin(phase * 3.7f));
        }

        // Advance main phase
        phase += phaseInc;
        if (phase >= 1.0f)
            phase -= 1.0f;

        float out = (noise * mix) + (tone * (1.0f - mix));
        return out * envAmp;
    }
};
