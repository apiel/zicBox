#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/MultiFx.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"
#include <cstring>

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
    float clickEnv = 1.0f;
    float lowPassState = 0.0f;
    float driveFeedback = 0.0f;
    char fxName[24] = "Off";

    float lerp(float a, float b, float t)
    {
        return a + t * (b - a);
    }

public:
    Param params[24] = {
        // --- PAGE 1: CORE SYNTHESIS ---
        { .label = "Duration", .unit = "ms", .value = 500.0f, .min = 50.0f, .max = 2500.0f },
        { .label = "Amp Env", .unit = "%", .value = 20.0f, .onUpdate = [](void* ctx, float val) { ((DrumKick23*)ctx)->envelopAmp.morph(val * 0.01f); } },
        { .label = "Sub Freq", .unit = "Hz", .value = 45.0f, .min = 30.0f, .max = 100.0f },
        { .label = "Sweep Dep", .unit = "%", .value = 80.0f },
        { .label = "Sweep Shp", .unit = "%", .value = 50.0f },
        { .label = "VCO Morph", .unit = "Tri-Sq", .value = 0.0f }, // Fixed: Morphs Sine to Square/Tri
        { .label = "Symmetry", .unit = "%", .value = 0.0f }, // Fixed: Works +/-
        { .label = "Hardness", .unit = "%", .value = 30.0f }, // Replaced Punch with Hardness (Waveshaper)
        { .label = "Click Amt", .unit = "%", .value = 20.0f },
        { .label = "Tone", .unit = "%", .value = 100.0f },
        { .label = "FM Dirt", .unit = "%", .value = 0.0f },
        { .label = "FM Ratio", .unit = "mult", .value = 1.618f, .min = 0.5f, .max = 8.0f, .step = 0.01f },

        // --- PAGE 2: FM & TEXTURE & POST ---
        { .label = "FM Depth", .unit = "%", .value = 0.0f },
        { .label = "Sub FM", .unit = "%", .value = 0.0f },
        { .label = "Drive", .unit = "%", .value = 20.0f },
        { .label = "Drive Shp", .unit = "%", .value = 50.0f }, // Fixed: Drastic feedback behavior
        { .label = "Compress", .unit = "%", .value = 10.0f },
        { .label = "Velo Sens", .unit = "%", .value = 50.0f }, // Fixed: Velocity modulation
        { .label = "FX Type", .string = fxName, .value = 0.0f, .max = (float)MultiFx::FX_COUNT - 1, .step = 1.0f, .precision = 0, .onUpdate = [](void* ctx, float v) { 
            auto e = (DrumKick23*)ctx; e->multiFx.setEffect(v); strcpy(e->fxName, e->multiFx.getEffectName()); } },
        { .label = "FX Amount", .unit = "%", .value = 0.0f },
        { .label = "" }, { .label = "" }, { .label = "" }, { .label = "" } // Placeholders for 24 total
    };

    // Param References
    Param& duration = params[0];
    Param& ampEnv = params[1];
    Param& subFreq = params[2];
    Param& sweepDep = params[3];
    Param& sweepShp = params[4];
    Param& vcoMorph = params[5];
    Param& symmetry = params[6];
    Param& hardness = params[7];
    Param& clickAmt = params[8];
    Param& tone = params[9];
    Param& fmDirt = params[10];
    Param& fmRatio = params[11];
    Param& fmDepth = params[12];
    Param& subFm = params[13];
    Param& drive = params[14];
    Param& driveShp = params[15];
    Param& compress = params[16];
    Param& veloSens = params[17];
    Param& fxType = params[18];
    Param& fxAmt = params[19];

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
        phase1 = 0.0f;
        phase2 = 0.0f;
        pitchEnv = 1.0f;
        clickEnv = 1.0f;
        driveFeedback = 0.0f;
        int totalSamples = static_cast<int>(sampleRate * (duration.value * 0.001f));
        envelopAmp.reset(totalSamples);
    }

    float sampleImpl()
    {
        float amp = envelopAmp.next();
        if (amp < 0.0001f) return 0.0f;

        // 1. Pitch Env
        float decayMod = (sweepShp.value * 0.01f + 0.35f) * 0.08f;
        pitchEnv *= Math::exp(-1.0f / (sampleRate * decayMod));
        float pCurved = pitchEnv * pitchEnv;
        float rootFreq = subFreq.value + (sweepDep.value * 6.0f * pCurved);

        // 2. Fixed FM Logic
        float modFreq = rootFreq * fmRatio.value;
        phase2 += modFreq * sampleRateDiv;
        if (phase2 > 1.0f) phase2 -= 1.0f;

        float dirt = fmDirt.value * 0.01f;
        float modSig = Math::fastSin(PI_X2 * phase2);
        // Add "Dirt" (rectification/folding)
        if (dirt > 0.0f) modSig = lerp(modSig, (modSig > 0 ? 1.0f : -1.0f), dirt * 0.5f);

        float totalFm = (fmDepth.value * 0.01f * pCurved) + (subFm.value * 0.01f);

        // 3. Oscillator & Morph
        phase1 += (rootFreq * sampleRateDiv) + (modSig * totalFm * 0.05f);
        if (phase1 > 1.0f) phase1 -= 1.0f;

        // VCO Morph: Sine -> Tri/Square hybrid
        float s = Math::fastSin(PI_X2 * phase1);
        float morphVal = vcoMorph.value * 0.01f;
        if (morphVal > 0.0f) {
            float sq = (s > 0 ? 1.0f : -1.0f);
            float tri = 4.0f * std::abs(phase1 - std::floor(phase1 + 0.5f)) - 1.0f;
            s = lerp(s, lerp(tri, sq, 0.5f), morphVal);
        }

        // Symmetry: Works across entire range now
        float sym = symmetry.value * 0.01f;
        s = s + sym * (1.0f - std::abs(s));

        // 4. Click & Hardness
        clickEnv *= Math::exp(-1.0f / (sampleRate * 0.003f)); // Fixed 3ms decay
        float sig = s + (Noise::sample() * clickEnv * clickAmt.value * 0.05f);

        // Hardness (Soft Clipper)
        float hrd = hardness.value * 0.01f * 2.0f;
        sig = std::tanh(sig * (1.0f + hrd));

        // 5. Enhanced Drive & Feedback Shape
        float drv = drive.value * 0.01f;
        float shp = driveShp.value * 0.01f;

        // Internal feedback loop for that "bass feedback" sound
        float feed = driveFeedback * shp * 0.6f;
        sig = applyDrive(sig + feed, drv * 3.0f);
        driveFeedback = sig; // Store for next sample

        // 6. Post processing
        float fCoeff = 0.05f + (tone.value * 0.01f) * 0.9f;
        lowPassState += fCoeff * (sig - lowPassState);
        sig = lowPassState;

        if (compress.value > 0.0f) sig = applyCompression(sig, compress.value * 0.01f);
        sig = multiFx.apply(sig, fxAmt.value * 0.01f);

        // Velocity Sensitivity
        float vSens = veloSens.value * 0.01f;
        float gain = (1.0f - vSens) + (velocity * vSens);

        return sig * amp * gain;
    }
};
