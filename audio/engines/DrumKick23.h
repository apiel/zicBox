#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/MultiFx.h"
#include "audio/effects/applyBoost.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"
#include <algorithm>
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
    float pitchEnv = 1.0f;
    float clickEnv = 1.0f;
    float noiseEnv = 1.0f;
    float fmEnv = 0.0f;
    float lowPassState = 0.0f;
    float driveFeedback = 0.0f;
    float bassBoostPrevInput = 0.0f;
    float bassBoostPrevOutput = 0.0f;
    char fxName[24] = "Off";

    float lerp(float a, float b, float t) { return a + t * (b - a); }

public:
    Param params[24] = {
        // --- PAGE 1: CORE ---
        { .label = "Duration", .unit = "ms", .value = 500.0f, .min = 50.0f, .max = 2500.0f, .step = 50.0f }, // 0
        { .label = "Amp Env", .unit = "%", .value = 20.0f, .onUpdate = [](void* ctx, float val) { ((DrumKick23*)ctx)->envelopAmp.morph(val * 0.01f); } }, // 1
        { .label = "Sub Freq", .unit = "Hz", .value = 45.0f, .min = 30.0f, .max = 100.0f }, // 2
        { .label = "Sweep Dep", .unit = "%", .value = 60.0f }, // 3
        { .label = "Sweep Spd", .unit = "%", .value = 30.0f }, // 4
        { .label = "Sweep Shp", .unit = "%", .value = 50.0f }, // 5
        { .label = "VCO Morph", .unit = "Tri-Sq", .value = 0.0f }, // 6
        { .label = "Symmetry", .unit = "%", .value = 0.0f }, // 7
        { .label = "Hardness", .unit = "%", .value = 30.0f }, // 8
        { .label = "Click Amt", .unit = "%", .value = 20.0f }, // 9
        { .label = "Noise Amt", .unit = "%", .value = 10.0f }, // 10
        { .label = "Noise Tim", .unit = "ms", .value = 20.0f, .min = 2.0f, .max = 200.0f }, // 11

        // --- PAGE 2: FM & DIST ---
        { .label = "FM", .unit = "%", .value = 0.0f }, // 12
        { .label = "FM Dirt", .unit = "Fold", .value = 0.0f }, // 13
        { .label = "FM Ratio", .unit = "mult", .value = 1.0f, .min = 0.5f, .max = 4.0f, .step = 0.01f }, // 14
        { .label = "FM Snap", .unit = "ms", .value = 25.0f, .min = 2.0f, .max = 200.0f }, // 15
        { .label = "Sub Harm", .unit = "%", .value = 0.0f }, // 16
        { .label = "Drive", .unit = "%", .value = 50.0f, .min = -100.0f }, // 17
        { .label = "Bass Boost", .unit = "%", .value = 50.0f }, // 18
        { .label = "Punch", .unit = "%", .value = 0.0f }, // 19 
        { .label = "Compress", .unit = "%", .value = 10.0f }, // 20
        { .label = "Tone", .unit = "%", .value = 100.0f }, // 21
        { .label = "FX Type", .string = fxName, .value = 0.0f, .max = (float)MultiFx::FX_COUNT - 1, .step = 1.0f, .onUpdate = [](void* ctx, float v) { 
            auto e = (DrumKick23*)ctx; e->multiFx.setEffect(v); strcpy(e->fxName, e->multiFx.getEffectName()); } }, // 22
        { .label = "FX Amount", .unit = "%", .value = 0.0f } // 23
    };

    // References
    Param& duration = params[0];
    Param& subFreq = params[2];
    Param& sweepDep = params[3];
    Param& sweepSpd = params[4];
    Param& sweepShp = params[5];
    Param& vcoMorph = params[6];
    Param& symmetry = params[7];
    Param& hardness = params[8];
    Param& clickAmt = params[9];
    Param& noiseAmt = params[10];
    Param& noiseTim = params[11];

    Param& fmDepth = params[12];
    Param& fmDirt = params[13];
    Param& fmRatio = params[14];
    Param& fmSnap = params[15];
    Param& subHarm = params[16];
    Param& drive = params[17];
    Param& bassBoost = params[18];
    Param& punch = params[19];
    Param& compress = params[20];
    Param& tone = params[21];
    Param& fxAmt = params[23];

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
        noiseEnv = 1.0f;
        fmEnv = 1.0f;
        driveFeedback = 0.0f;
        envelopAmp.reset(static_cast<int>(sampleRate * (duration.value * 0.001f)));
    }

    float sampleImpl()
    {
        float amp = envelopAmp.next();
        if (amp < 0.0001f) return 0.0f;

        // 1. PITCH & FM ENVELOPES
        float spd = lerp(0.005f, 0.15f, (100.0f - sweepSpd.value) * 0.01f);
        pitchEnv *= Math::exp(-1.0f / (sampleRate * spd));
        float pMorph = lerp(pitchEnv, pitchEnv * pitchEnv, sweepShp.value * 0.01f);
        float rootFreq = subFreq.value + (sweepDep.value * 4.0f * pMorph);

        // FM Snap (Decay for FM attack)
        fmEnv *= Math::exp(-1.0f / (sampleRate * (fmSnap.value * 0.001f)));

        // 2. MODULATOR (Phase 2)
        phase2 += (rootFreq * fmRatio.value) * sampleRateDiv;
        if (phase2 > 1.0f) phase2 -= 1.0f;

        float modSig = Math::fastSin(PI_X2 * phase2);

        // FM Dirt (Wavefolding)
        float foldAmt = fmDirt.value * 0.01f;
        float thresh = 1.0f - (foldAmt * 0.6f);
        if (modSig > thresh) modSig = thresh - (modSig - thresh);
        else if (modSig < -thresh) modSig = -thresh - (modSig + thresh);
        modSig *= (1.0f / thresh);

        // 3. CARRIER OSCILLATOR (Phase 1)
        float fmModulation = (modSig * (fmDepth.value * 0.001f) * fmEnv * 0.2f);
        phase1 += (rootFreq * sampleRateDiv) + fmModulation;
        if (phase1 > 1.0f) phase1 -= 1.0f;

        float s = Math::fastSin(PI_X2 * phase1);

        // VCO Morph (Sine -> Tri -> Square)
        float morph = vcoMorph.value * 0.01f;
        if (morph > 0.0f) {
            float tri = 2.0f * std::abs(2.0f * (phase1 - std::floor(phase1 + 0.5f))) - 1.0f;
            float sq = (s > 0.0f) ? 0.7f : -0.7f;
            if (morph < 0.5f) s = lerp(s, tri, morph * 2.0f);
            else s = lerp(tri, sq, (morph - 0.5f) * 2.0f);
        }

        // Symmetry & Sub Harmonic
        s += (symmetry.value * 0.01f) * (1.0f - std::abs(s));
        if (subHarm.value > 0.0f) s += Math::fastSin(PI_X2 * phase1 * 0.5f) * (subHarm.value * 0.01f);

        // 4. TRANSIENTS (Click + Noise)
        clickEnv *= Math::exp(-1.0f / (sampleRate * 0.002f));
        noiseEnv *= Math::exp(-1.0f / (sampleRate * noiseTim.value * 0.001f));
        float sig = s + (Noise::sample() * clickEnv * clickAmt.value * 0.12f)
            + (Noise::sample() * noiseEnv * noiseAmt.value * 0.04f);

        // 5. DISTORTION STACK
        // Punch (Initial transient boost)
        sig *= (1.0f + punch.value * 0.01f * clickEnv);

        // Hardness
        sig *= (1.0f + hardness.value * 0.1f);

        // Drive (Positive = Feedback/Analog, Negative = Standard Clip)
        if (drive.value > 0.0f) {
            sig = applyDriveFeedback(sig, drive.value * 0.01f, driveFeedback);
        } else {
            sig = applyDrive(sig, drive.value * -0.05f);
        }

        // Bass Boost
        sig = applyBoost(sig, bassBoost.value * 0.01f, bassBoostPrevInput, bassBoostPrevOutput);

        // 6. POST
        float fCoeff = (tone.value * 0.01f) * 0.5f;
        lowPassState += fCoeff * (sig - lowPassState);
        sig = lerp(sig, lowPassState, 0.6f);

        if (compress.value > 0.0f) sig = applyCompression(sig, compress.value * 0.01f);
        sig = multiFx.apply(sig, fxAmt.value * 0.01f);

        return sig * amp * velocity;
    }
};