#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/MultiFx.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"

class DrumImpact : public EngineBase<DrumImpact> {
public:
    EnvelopDrumAmp envelopAmp;
    MultiFx multiFx;

protected:
    const float sampleRate;
    float velocity = 1.0f;

    float phase1 = 0.0f;
    float phase2 = 0.0f;
    float pitchEnv = 1.0f;

    // Improved Click: Resonant filtered pulse instead of raw noise
    float clickFilter = 0.0f;
    float clickEnv = 0.0f;

    char fxName[24] = "Off";

    // Smart Morph: Sine -> Triangle -> Square
    inline float getSmartWav(float phase, float morph)
    {
        float s = Math::fastSin(PI_X2 * phase);
        float tri = 4.0f * (phase < 0.5f ? phase : 1.0f - phase) - 1.0f;

        float sq = s > 0 ? 0.85f : -0.85f;
        return tri * (1.0f - morph) + sq * morph;
    }

public:
    Param params[12] = {
        { .label = "Duration", .unit = "ms", .value = 500.0f, .min = 50.0f, .max = 2500.0f },
        { .label = "Amp Env", .unit = "%", .value = 20.0f, .onUpdate = [](void* ctx, float val) { ((DrumImpact*)ctx)->envelopAmp.morph(val * 0.01f); } },
        { .label = "Body Freq", .unit = "Hz", .value = 45.0f, .min = 30.0f, .max = 100.0f },
        { .label = "Sweep Depth", .unit = "%", .value = 80.0f },
        { .label = "Sweep Shape", .value = 0.0f, .min = 0.0f, .max = 100.0f },
        { .label = "VCO Morph", .unit = "Tri-Sq", .value = 0.0f },
        { .label = "FM", .unit = "%", .value = 0.0f }, // Smoother Ratio logic inside
        { .label = "Click Type", .unit = "Hz", .value = 1500.0f, .min = 500.0f, .max = 5000.0f, .step = 100.0f },
        { .label = "Click", .unit = "%", .value = 20.0f },
        { .label = "Drive", .unit = "%", .value = 20.0f },
        { .label = "FX type", .string = fxName, .value = 0.0f, .max = MultiFx::FX_COUNT - 1, .onUpdate = [](void* ctx, float val) {
             auto edge = (DrumImpact*)ctx;
             edge->multiFx.setEffect(val);
             strcpy(edge->fxName, edge->multiFx.getEffectName());
         } },
        { .label = "FX amount", .unit = "%", .value = 0.0f },
    };

    Param& duration = params[0];
    Param& ampEnv = params[1];
    Param& bodyFreq = params[2];
    Param& sweepDepth = params[3];
    Param& sweepShape = params[4];
    Param& vcoMorph = params[5];
    Param& fmTexture = params[6];
    Param& clickFreq = params[7];
    Param& clickAmt = params[8];
    Param& drive = params[9];
    Param& fxType = params[10];
    Param& fxAmount = params[11];

    DrumImpact(const float sampleRate, float* fxBuffer)
        : EngineBase(Drum, "Impact", params)
        , multiFx(sampleRate, fxBuffer)
        , sampleRate(sampleRate)
    {
        init();
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        phase1 = phase2 = 0.0f;
        pitchEnv = 1.0f;
        clickEnv = 1.0f;
        clickFilter = 0.0f;

        int totalSamples = static_cast<int>(sampleRate * (params[0].value * 0.001f));
        envelopAmp.reset(totalSamples);
    }

    float sampleImpl()
    {
        float amp = envelopAmp.next();
        if (amp < 0.0001f) return 0.0f;

        // 1. Pitch Env with fixed range scaling
        float decayMod = (sweepShape.value * 0.01f + 0.35f) * 0.08f;
        pitchEnv *= Math::exp(-1.0f / (sampleRate * decayMod));

        float freq1 = params[2].value + (pct(params[3]) * 600.0f * (pitchEnv * pitchEnv));

        float freq2 = freq1 * (1.618f + fmTexture.value * 0.005f);

        float vcoMorphPct = vcoMorph.value * 0.01f;

        phase2 += freq2 / sampleRate;
        if (phase2 > 1.0f) phase2 -= 1.0f;
        float modulator = getSmartWav(phase2, vcoMorphPct);

        // Linear PM is smoother than raw FM
        float pmMod = modulator * fmTexture.value * 0.00015f;
        phase1 += (freq1 / sampleRate) + pmMod;
        if (phase1 > 1.0f) phase1 -= 1.0f;

        float sig = getSmartWav(phase1, vcoMorphPct);

        // 3. New "Tonal" Click
        // High-passed noise pulse creates a "skin" hit rather than white noise trash
        clickEnv *= Math::exp(-1.0f / (sampleRate * 0.003f));
        float n = Noise::sample() * clickEnv;
        clickFilter += 0.01f * (n - clickFilter); // Simple HPF
        sig += (n - clickFilter) * clickAmt.value * 0.05f;

        // 4. Drive
        if (drive.value > 0.0f) {
            sig = applyDrive(sig, pct(drive) * 3.0f);
        }

        float out = multiFx.apply(sig, fxAmount.value * 0.01f);

        return out * amp * velocity;
    }
};