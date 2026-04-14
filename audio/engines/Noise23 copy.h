#pragma once

#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"
#include "helpers/clamp.h"
#include <algorithm>
#include <cmath>

class Noise23 : public EngineBase<Noise23> {
public:
    static constexpr int REVERB_BUF_SIZE = 16384;

protected:
    const float sampleRate;
    float* delayBuf = nullptr;
    float* reverbBuf = nullptr;

    FilterSVF mainFilter; // For the "Body" (Wind/Waves)
    FilterSVF detailFilter; // For the "Tone" (Whistles/Bubbles)

    // Modulation
    float turbulence = 0.0f;
    float turbTarget = 0.0f;
    float shValue = 0.0f;
    uint32_t shCounter = 0;
    float lfoPhase = 0.0f;

    // Envelope
    float ampEnv = 0.0f;
    float ampStep = 0.0f;

    // Internal Noise States
    float pinkState = 0.0f;
    float brownState = 0.0f;

    bool isOn = false;

    // Local lerp since it's missing from your Math namespace
    static float lerp(float a, float b, float t) { return a + t * (b - a); }

public:
    Param params[16];

    // PAGE 1: GLOBAL & TIMBRE
    Param& duration = addParam({ .key = "dur", .label = "Release", .unit = "ms", .value = 1000.0f, .max = 10000.0f });
    Param& noiseMorph = addParam({ .key = "morph", .label = "Noise Morph", .unit = "W-P-B", .value = 50.0f });
    Param& grit = addParam({ .key = "grit", .label = "Grit/Saturate", .unit = "%", .value = 0.0f });
    Param& masterGain = addParam({ .key = "gain", .label = "Gain", .unit = "%", .value = 70.0f });

    // PAGE 2: TURBULENCE (The "Wind" Engine)
    Param& turbRate = addParam({ .key = "tRate", .label = "Turb Rate", .unit = "Hz", .value = 0.5f, .max = 10.0f });
    Param& turbDepth = addParam({ .key = "tDepth", .label = "Turb Depth", .unit = "%", .value = 40.0f });
    Param& gusting = addParam({ .key = "gust", .label = "Gusting", .unit = "%", .value = 20.0f });

    // PAGE 3: PARTICLES (The "Water" Engine)
    Param& dripRate = addParam({ .key = "dRate", .label = "Drip Rate", .unit = "Hz", .value = 15.0f, .max = 200.0f });
    Param& dripRand = addParam({ .key = "dRand", .label = "Drip Chaos", .unit = "%", .value = 50.0f });
    Param& dripRes = addParam({ .key = "dRes", .label = "Drip Tone", .unit = "%", .value = 85.0f });

    // PAGE 4: DUAL FILTERS
    Param& f1Cutoff = addParam({ .key = "f1Cut", .label = "Body Cut", .unit = "%", .value = 30.0f });
    Param& f1Res = addParam({ .key = "f1Res", .label = "Body Res", .unit = "%", .value = 10.0f });
    Param& f2Cutoff = addParam({ .key = "f2Cut", .label = "Peak Cut", .unit = "%", .value = 60.0f });
    Param& f2Res = addParam({ .key = "f2Res", .label = "Peak Res", .unit = "%", .value = 40.0f });
    Param& fBalance = addParam({ .key = "fBal", .label = "F1 <-> F2", .unit = "%", .value = 50.0f });

    // PAGE 5: SPACE
    Param& rvbMix = addParam({ .key = "rvb", .label = "Outdoor Mix", .unit = "%", .value = 30.0f });

    // Fixed Constructor to match studio.cpp requirements
    Noise23(float sr, float* dlBuf, float* rvBuf, float* fxBuf)
        : EngineBase(Synth, "Noise23", params)
        , sampleRate(sr)
        , delayBuf(dlBuf)
        , reverbBuf(rvBuf)
    {
    }

    void noteOnImpl(uint8_t note, float vel)
    {
        ampEnv = 1.0f;
        ampStep = 1.0f / (sampleRate * (duration.value * 0.001f));
        lfoPhase = 0.0f;
        shCounter = 0;
        isOn = true;
    }

    void noteOffImpl(uint8_t)
    {
        // Optional: you could make it go to a release phase here
        isOn = false;
    }

    float getMorphedNoise(float m)
    {
        float w = (float)Noise::sample();
        pinkState = 0.997f * pinkState + w * 0.05f;
        brownState = 0.999f * brownState + w * 0.01f;
        if (m < 0.5f) return lerp(w, pinkState * 2.0f, m * 2.0f);
        return lerp(pinkState * 2.0f, brownState * 8.0f, (m - 0.5f) * 2.0f);
    }

    float sampleImpl()
    {
        // return 0.0f;
        if (ampEnv <= 0.0f) return 0.0f;

        if (!isOn) ampEnv -= ampStep;

        // 1. TURBULENCE
        lfoPhase += turbRate.value / sampleRate;
        if (lfoPhase > 1.0f) lfoPhase -= 1.0f;
        float baseLfo = std::sin(2.0f * M_PI * lfoPhase);

        if ((float)Noise::sample() > (0.999f - gusting.value * 0.0001f)) turbTarget = (float)Noise::sample();
        turbulence += (turbTarget - turbulence) * 0.001f;
        float modValue = (baseLfo + turbulence) * (turbDepth.value * 0.01f);

        // 2. PARTICLES
        float dripTarget = (sampleRate / std::max(0.1f, dripRate.value));
        if (++shCounter > dripTarget * (1.0f + (float)Noise::sample() * dripRand.value * 0.01f)) {
            shValue = (float)Noise::sample();
            shCounter = 0;
        }

        // 3. SOURCE
        float src = getMorphedNoise(noiseMorph.value * 0.01f);

        // 4. FILTERS
        float c1 = CLAMP(f1Cutoff.value * 0.01f + modValue * 0.3f, 0.01f, 0.99f);
        mainFilter.setCutoff(c1);
        mainFilter.setResonance(f1Res.value * 0.01f);
        float body = mainFilter.process12(src).lp;

        float c2 = CLAMP(f2Cutoff.value * 0.01f + shValue * (dripRand.value * 0.01f), 0.01f, 0.99f);
        detailFilter.setCutoff(c2);
        detailFilter.setResonance(lerp(f2Res.value * 0.01f, 0.99f, dripRes.value * 0.01f));
        float detail = detailFilter.process12(src).bp;

        // 5. MIX
        float sig = lerp(body, detail, fBalance.value * 0.01f);
        if (grit.value > 0.0f) sig = std::tanh(sig * (1.0f + grit.value * 0.1f));

        sig *= ampEnv * (masterGain.value * 0.01f);

        // Note: Reverb process call usually happens in bufferedFxProcess or similar
        // in your architecture, but for now we return the dry-ish signal.
        return sig;
    }
};