#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/MultiFx.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"

class DrumKick23 : public EngineBase<DrumKick23> {
public:
    EnvelopDrumAmp envelopAmp;
    MultiFx multiFx;

protected:
    const float sampleRate;
    const float sampleRateDiv;
    float velocity = 1.0f;
    float phase1 = 0.0f;
    float phase2 = 0.0f;
    float pitchEnv = 0.0f;
    float clickEnv = 0.0f;
    float lowPassState = 0.0f;
    char fxName[24] = "Off";

public:
    Param params[24] = {
        // --- PAGE 1: CORE SYNTHESIS ---
        { .label = "Duration", .unit = "ms", .value = 500.0f, .min = 50.0f, .max = 2500.0f },
        { .label = "Amp Env", .unit = "%", .value = 20.0f, .onUpdate = [](void* ctx, float val) { ((DrumKick23*)ctx)->envelopAmp.morph(val * 0.01f); } },
        { "Sub Freq", "Hz", .value = 45.0f, .min = 30.0f, .max = 100.0f }, // 2
        { "Sweep Dep", "%", .value = 80.0f }, // 3
        { "Sweep Shp", "%", .value = 50.0f }, // 4
        { "VCO Morph", "Tri-Sq", .value = 0.0f }, // 5
        { .label = "Symmetry", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Punch", .unit = "%", .value = 30.0f },
        { "Click Amt", "%", .value = 20.0f }, // 8
        { "Click Tim", "ms", .value = 3.0f, .min = 0.5f, .max = 20.0f }, // 9
        { "Tone", "%", .value = 100.0f }, // 10 (Filter)
        { "VCO Phase", "deg", .value = 0.0f, .min = 0.0f, .max = 360.0f }, // 11

        // --- PAGE 2: FM & TEXTURE & POST ---
        { .label = "FM Dirt", .unit = "%", .value = 0.0f, .min = -100.0f }, // 12
        { "FM Ratio", "mult", .value = 1.618f, 0.5f, 8.0f, 0.01f }, // 13
        { "FM Depth", "%", .value = 0.0f }, // 14
        { "Sub FM", "%", .value = 0.0f }, // 15 (Clean FM)
        { "Drive", "%", .value = 20.0f }, // 16
        { "Drive Shp", "%", .value = 50.0f }, // 17
        { "Compress", "%", .value = 10.0f }, // 18
        { "FX Type", "", .string = fxName, .value = 0.0f, 0.0f, (float)MultiFx::FX_COUNT - 1, 1.0f, 0, .onUpdate = [](void* ctx, float v) { 
            auto e = (DrumKick23*)ctx; e->multiFx.setEffect(v); strcpy(e->fxName, e->multiFx.getEffectName()); } },
        { "FX Amount", "%", .value = 0.0f }, // 20
        { "Width", "%", .value = 0.0f }, // 21
        { "Master", "%", .value = 100.0f }, // 22
        { "Velo Sens", "%", .value = 50.0f } // 23
    };

    DrumKick23(const float sampleRate, float* fxBuffer)
        : EngineBase(Drum, "Kick23", params)
        , multiFx(sampleRate, fxBuffer)
        , sampleRate(sampleRate)
        , sampleRateDiv(1.0f / sampleRate)
    {
        init();
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        phase1 = params[11].value / 360.0f; // Initial phase
        phase2 = 0.0f;
        pitchEnv = 1.0f;
        clickEnv = 1.0f;
        int totalSamples = static_cast<int>(sampleRate * (params[0].value * 0.001f));
        envelopAmp.reset(totalSamples);
    }

    float sampleImpl()
    {
        float amp = envelopAmp.next();
        if (amp < 0.0001f) return 0.0f;

        // 1. Pitch Envelope (Kick2/Impact hybrid)
        float decayMod = (params[4].value * 0.01f + 0.35f) * 0.08f;
        pitchEnv *= Math::exp(-1.0f / (sampleRate * decayMod));
        float pCurved = pitchEnv * pitchEnv;

        float rootFreq = params[2].value + (params[3].value * 6.0f * pCurved);

        // 2. FM Logic
        float dirt = params[12].value * 0.01f;
        float cleanFm = params[15].value * 0.01f;
        float modFreq = rootFreq * params[13].value;

        phase2 += modFreq * sampleRateDiv;
        if (phase2 > 1.0f) phase2 -= 1.0f;

        // Dirt uses a Triangle modulator (nasty), Clean uses Sine
        float modulator = (Math::fastSin(PI_X2 * phase2) * (1.0f - std::abs(dirt))) + ((4.0f * (phase2 < 0.5f ? phase2 : 1.0f - phase2) - 1.0f) * dirt);

        float fmDepth = (params[14].value * 0.01f * pCurved) + (cleanFm * 0.2f);

        // 3. Oscillator
        phase1 += (rootFreq * sampleRateDiv) + (modulator * fmDepth * 0.05f);
        if (phase1 > 1.0f) phase1 -= 1.0f;

        // Symmetry/Morph logic
        float sig = Math::fastSin(PI_X2 * phase1);
        float sym = params[6].value * 0.009f;
        sig = (sig + sym * (sig * sig * sig)) / (1.0f + sym);

        // 4. Transient/Click
        clickEnv *= Math::exp(-1.0f / (sampleRate * params[9].value * 0.001f));
        sig += Noise::sample() * clickEnv * params[8].value * 0.01f;

        // 5. Punch Drive (Immediate saturation)
        if (params[7].value > 0.0f) {
            float pnt = (params[7].value * 0.01f) * 10.0f * pCurved;
            sig = (sig * (1.0f + pnt)) / (1.0f + pnt * std::abs(sig));
        }

        // 6. Post FX
        if (params[16].value > 0.0f) sig = applyDrive(sig, params[16].value * 0.01f * 2.0f);

        float fCoeff = 0.05f + (params[10].value * 0.01f) * 0.9f;
        lowPassState += fCoeff * (sig - lowPassState);
        sig = lowPassState;

        if (params[18].value > 0.0f) sig = applyCompression(sig, params[18].value * 0.01f);

        sig = multiFx.apply(sig, params[20].value * 0.01f);

        float veloGain = (1.0f - params[23].value * 0.01f) + (velocity * params[23].value * 0.01f);
        return sig * amp * veloGain * (params[22].value * 0.01f);
    }
};