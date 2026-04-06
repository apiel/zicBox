#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/MultiFx.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"
#include "helpers/clamp.h"

class DrumImpact : public EngineBase<DrumImpact> {
public:
    EnvelopDrumAmp envelopAmp;
    MultiFx multiFx;

protected:
    const float sampleRate;
    const float sampleRateDiv;
    float velocity = 1.0f;

    float phase1 = 0.0f;
    float phase2 = 0.0f;
    float pitchEnv = 1.0f;

    float clickFilter = 0.0f;
    float clickEnv = 0.0f;

    char fxName[24] = "Off";

    // Smart Morph: Sine -> Triangle -> Square
    inline float getSmartWav(float phase, float morph)
    {
        float m = CLAMP(morph, 0.0f, 1.0f);
        float s = Math::fastSin(PI_X2 * phase);
        float tri = 4.0f * (phase < 0.5f ? phase : 1.0f - phase) - 1.0f;

        // Simple morph between Triangle and a clipped Sine (Square-ish)
        float sq = s > 0 ? 0.85f : -0.85f;
        return tri * (1.0f - m) + sq * m;
    }

public:
    Param params[12];

    Param& duration = addParam({ .key = "duration", .label = "Duration", .unit = "ms", .value = 500.0f, .min = 50.0f, .max = 2500.0f });
    Param& ampEnv = addParam({ .key = "ampEnv", .label = "Amp Env", .unit = "%", .value = 20.0f, .onUpdate = [](void* ctx, float val) {
                                  ((DrumImpact*)ctx)->envelopAmp.morph(val * 0.01f);
                              } });

    Param& bodyFreq = addParam({ .key = "bodyFreq", .label = "Body Freq", .unit = "Hz", .value = 45.0f, .min = 30.0f, .max = 100.0f });
    Param& sweepDepth = addParam({ .key = "sweepDepth", .label = "Sweep Depth", .unit = "%", .value = 80.0f });
    Param& sweepShape = addParam({ .key = "sweepShape", .label = "Sweep Shape", .value = 50.0f, .min = 0.0f, .max = 100.0f });

    Param& vcoMorph = addParam({ .key = "vcoMorph", .label = "VCO Morph", .unit = "Tri-Sq", .value = 0.0f });
    Param& texture = addParam({ .key = "texture", .label = "Texture", .unit = "%", .value = 10.0f });
    Param& vcoRatio = addParam({ .key = "vcoRatio", .label = "VCO Ratio", .unit = "mult", .value = 1.618f, .min = 0.5f, .max = 4.0f, .step = 0.01f });

    Param& clickAmt = addParam({ .key = "clickAmt", .label = "Click", .unit = "%", .value = 20.0f });
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 20.0f });

    Param& fxType = addParam({ .key = "fxType", .label = "FX type", .string = fxName, .value = 0.0f, .max = (float)MultiFx::FX_COUNT - 1, .onUpdate = [](void* ctx, float val) {
                                  auto edge = (DrumImpact*)ctx;
                                  edge->multiFx.setEffect(val);
                                  strcpy(edge->fxName, edge->multiFx.getEffectName());
                              } });
    Param& fxAmount = addParam({ .key = "fxAmount", .label = "FX amount", .unit = "%", .value = 0.0f });

    DrumImpact(const float sampleRate, float* fxBuffer)
        : EngineBase(Drum, "Impact", params)
        , multiFx(sampleRate, fxBuffer)
        , sampleRate(sampleRate)
        , sampleRateDiv(1.0f / sampleRate)
    {
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        phase1 = 0.0f;
        phase2 = 0.25f; // Initial phase offset for sharper transient
        pitchEnv = 1.0f;
        clickEnv = 1.0f;
        clickFilter = 0.0f;

        int totalSamples = static_cast<int>(sampleRate * (duration.value * 0.001f));
        envelopAmp.reset(totalSamples);
    }

    float sampleImpl()
    {
        float amp = envelopAmp.next();
        if (amp < 0.0001f) return 0.0f;

        // --- 1. Pitch Envelope ---
        float decayMod = (sweepShape.value * 0.01f + 0.35f) * 0.08f;
        pitchEnv *= Math::exp(-1.0f / (sampleRate * decayMod));
        float pCurved = pitchEnv * pitchEnv;

        float freq1 = bodyFreq.value + (pct(sweepDepth) * 600.0f * pCurved);
        // float freq1 = bodyFreq.value + (sweepDepth.value * 6.0f * pCurved);
        float freq2 = freq1 * vcoRatio.value;

        // --- 2. Dual VCO PM Engine ---
        float vcoMorphPct = vcoMorph.value * 0.01f;

        // Modulator (VCO2)
        phase2 += freq2 * sampleRateDiv;
        if (phase2 > 1.0f) phase2 -= 1.0f;
        float modulator = getSmartWav(phase2, vcoMorphPct - 0.15f);

        // Carrier (VCO1) with damped PM (only affects the attack)
        float pmMod = modulator * texture.value * 0.00015f * pitchEnv;
        phase1 += (freq1 * sampleRateDiv) + pmMod;
        if (phase1 > 1.0f) phase1 -= 1.0f;

        float sig = getSmartWav(phase1, vcoMorphPct + 0.15f);

        // Mix a bit of the modulator back in for "Texture"
        float mixAmt = texture.value * 0.01f;
        // sig = (sig * (1.0f - mixAmt)) + (modulator * mixAmt * 0.4f);
        sig = (sig * (1.0f - mixAmt)) + (modulator * (mixAmt * 0.7f + 0.3f));

        // --- 3. Tonal Click ---
        clickEnv *= Math::exp(-1.0f / (sampleRate * 0.003f));
        float n = Noise::sample() * clickEnv;
        // Simple High Pass Filter for the noise
        clickFilter += 0.1f * (n - clickFilter);
        sig += (n - clickFilter) * clickAmt.value * 0.06f;

        // --- 4. Drive ---
        if (drive.value > 0.0f) {
            sig = applyDrive(sig, pct(drive) * 3.0f);
        }

        // --- 5. FX and Final Output ---
        float out = multiFx.apply(sig, fxAmount.value * 0.01f);

        return out * amp * velocity;
    }
};