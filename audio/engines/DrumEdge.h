#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include "audio/MultiFx.h"

class DrumEdge : public EngineBase<DrumEdge> {
public:
    EnvelopDrumAmp envelopAmp;
    MultiFx multiFx;

protected:
    const float sampleRate;
    float velocity = 1.0f;

    float phase1 = 0.0f;
    float phase2 = 0.0f;
    float pitchEnv = 1.0f; // Fast exponential decay for the VCOs
    float lowPassState = 0.0f;

    char fxName[24] = "Off";

    // Helper to generate morphable Tri -> Square
    // Phase is 0..1. Morph is 0..1
    inline float getWav(float phase, float morph)
    {
        // Triangle: basic 0->1->0 ramp
        float tri = 4.0f * (phase < 0.5f ? phase : 1.0f - phase) - 1.0f;
        // Square: using a soft-clipper on a high-gain triangle is smoother/cheaper
        if (morph < 0.01f) return tri;

        float square = tri > 0 ? 1.0f : -1.0f;
        return tri * (1.0f - morph) + square * morph;
    }

public:
    Param params[12] = {
        { .label = "Duration", .unit = "ms", .value = 600.0f, .min = 10.0f, .max = 2000.0f },
        { .label = "Amp Env", .unit = "%", .value = 0.0f, .onUpdate = [](void* ctx, float val) { ((DrumEdge*)ctx)->envelopAmp.morph(val * 0.01f); } },
        { .label = "VCO1 Freq", .unit = "Hz", .value = 40.0f, .min = 20.0f, .max = 400.0f },
        { .label = "VCO1 Wave", .unit = "tri-sq", .value = 0.0f },
        { .label = "VCO1 Env", .unit = "%", .value = 50.0f },
        { .label = "VCO2 Freq", .unit = "Hz", .value = 42.0f, .min = 20.0f, .max = 800.0f },
        { .label = "VCO2 Wave", .unit = "tri-sq", .value = 0.0f },
        { .label = "VCO2 Env", .unit = "%", .value = 20.0f },
        { .label = "FM 1-2", .unit = "%", .value = 10.0f },
        { .label = "VCO Mix", .unit = "1-2", .value = 50.0f },
        { .label = "FX type", .string = fxName, .value = 0.0f, .max = MultiFx::FX_COUNT - 1, .onUpdate = [](void* ctx, float val) { 
            auto edge = (DrumEdge*)ctx;
            edge->multiFx.setEffect(val);
            strcpy(edge->fxName, edge->multiFx.getEffectName());
        } },
        { .label = "FX amount", .unit = "%", .value = 0.0f },
    };

    // Easy access pointers
    Param& duration = params[0];
    Param& ampEnv = params[1];
    Param& vco1Freq = params[2];
    Param& vco1Shape = params[3];
    Param& vco1EnvAmt = params[4];
    Param& vco2Freq = params[5];
    Param& vco2Shape = params[6];
    Param& vco2EnvAmt = params[7];
    Param& fmAmount = params[8];
    Param& vcoMix = params[9];
    Param& fxType = params[10];
    Param& fxAmount = params[11];

    DrumEdge(const float sampleRate)
        : EngineBase(Drum, "Edge", params)
        , multiFx(sampleRate)
        , sampleRate(sampleRate)
    {
        init();
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        phase1 = phase2 = 0.0f;
        pitchEnv = 1.0f;
        lowPassState = 0.0f;
        int totalSamples = static_cast<int>(sampleRate * (duration.value * 0.001f));
        envelopAmp.reset(totalSamples);
    }

    float sampleImpl()
    {
        float amp = envelopAmp.next();
        if (amp < 0.0001f) return 0.0f;

        // 1. Update pitch envelope (Fast decay for that Edge "knock")
        pitchEnv *= Math::exp(-1.0f / (sampleRate * 0.04f));

        // 2. Calculate Frequencies with Env
        // The Edge has a massive pitch sweep range
        float f1 = vco1Freq.value + (pct(vco1EnvAmt) * 1000.0f * pitchEnv);
        float f2 = vco2Freq.value + (pct(vco2EnvAmt) * 1200.0f * pitchEnv);

        // 3. VCO 1 Logic
        phase1 += f1 / sampleRate;
        if (phase1 > 1.0f) phase1 -= 1.0f;
        float out1 = getWav(phase1, pct(vco1Shape));

        // 4. VCO 2 Logic with FM (VCO1 modulates VCO2)
        // fmAmount 0..1 scales up to a high modulation index
        float fmMod = out1 * pct(fmAmount) * 0.15f;
        phase2 += (f2 / sampleRate) + fmMod;
        if (phase2 > 1.0f) phase2 -= 1.0f;
        if (phase2 < 0.0f) phase2 += 1.0f; // FM can push phase negative
        float out2 = getWav(phase2, pct(vco2Shape));

        // 5. Mix and Process
        float mixer = pct(vcoMix);
        float sig = (out1 * (1.0f - mixer)) + (out2 * mixer);

        float out = multiFx.apply(sig, fxAmount.value * 0.01f);
        return out * amp * velocity;
    }
};