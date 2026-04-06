#pragma once

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
    MultiFx multiFx;

protected:
    const float sampleRate;
    const float sampleRateDiv;
    float velocity = 1.0f;
    float phase1 = 0.0f;
    float phase2 = 0.0f;
    float phaseVCO2 = 0.0f;
    float pitchEnv = 1.0f;
    float clickEnv = 1.0f;
    float noiseEnv = 1.0f;
    float fmEnv = 0.0f;
    float ampEnv = 0.0f;
    float ampStep = 0.0f;
    float lowPassState = 0.0f;
    float driveFeedback = 0.0f;
    float bassBoostPrevInput = 0.0f;
    float bassBoostPrevOutput = 0.0f;
    float compressionState = 0.0f;
    float notePitchMod = 1.0f;
    char fxName[24] = "Off";

    float lerp(float a, float b, float t) { return a + t * (b - a); }

public:
    Param params[24];

    Param& duration = addParam({ .key = "duration", .label = "Duration", .unit = "ms", .value = 500.0f, .min = 50.0f, .max = 2500.0f, .step = 50.0f });
    Param& subFreq = addParam({ .key = "subFreq", .label = "Sub Freq", .unit = "Hz", .value = 45.0f, .min = 30.0f, .max = 100.0f });
    Param& vcoMorph = addParam({ .key = "vcoMorph", .label = "VCO Morph", .unit = "Tri-Sq", .value = 0.0f });
    Param& sweepDep = addParam({ .key = "sweepDep", .label = "Sweep Dep", .unit = "%", .value = 60.0f });
    Param& sweepSpd = addParam({ .key = "sweepSpd", .label = "Sweep Spd", .unit = "%", .value = 30.0f });
    Param& sweepShp = addParam({ .key = "sweepShp", .label = "Sweep Shp", .unit = "%", .value = 50.0f });
    Param& v2Level = addParam({ .key = "v2Level", .label = "V2 Level", .unit = "%", .value = 0.0f });
    Param& v2Harm = addParam({ .key = "v2Harm", .label = "V2 Harm", .unit = "index", .value = 2.0f, .min = 1.0f, .max = 12.0f, .step = 1.0f });
    Param& v2Morph = addParam({ .key = "v2Morph", .label = "V2 Morph", .unit = "Fold", .value = 0.0f });
    Param& subHarm = addParam({ .key = "subHarm", .label = "Sub Harm", .unit = "%", .value = 0.0f });
    Param& hardness = addParam({ .key = "hardness", .label = "Hardness", .unit = "%", .value = 10.0f });
    Param& clickAmt = addParam({ .key = "clickAmt", .label = "Click Amt", .unit = "%", .value = 20.0f });

    Param& fmDepth = addParam({ .key = "fmDepth", .label = "FM", .unit = "%", .value = 0.0f });
    Param& fmDirt = addParam({ .key = "fmDirt", .label = "FM Dirt", .unit = "Fold", .value = 0.0f });
    Param& fmRatio = addParam({ .key = "fmRatio", .label = "FM Ratio", .unit = "mult", .value = 1.0f, .min = 0.5f, .max = 4.0f, .step = 0.01f });
    Param& fmSnap = addParam({ .key = "fmSnap", .label = "FM Snap", .unit = "ms", .value = 25.0f, .min = 2.0f, .max = 200.0f });
    Param& noiseAmt = addParam({ .key = "noiseAmt", .label = "Noise Amt", .unit = "%", .value = 10.0f });
    Param& noiseTim = addParam({ .key = "noiseTim", .label = "Noise Tim", .unit = "ms", .value = 20.0f, .min = 2.0f, .max = 200.0f });
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 0.0f, .min = -100.0f });
    Param& bassBoost = addParam({ .key = "bassBoost", .label = "Bass Boost", .unit = "%", .value = 30.0f });
    Param& compress = addParam({ .key = "compress", .label = "Compress", .unit = "%", .value = 100.0f });
    Param& tone = addParam({ .key = "tone", .label = "Tone", .unit = "%", .value = 100.0f });
    Param& fxType = addParam({ .key = "fxType", .label = "FX Type", .string = fxName, .value = 0.0f, .max = (float)MultiFx::FX_COUNT - 1, .step = 1.0f, // Skip Format
        .onUpdate = [](void* ctx, float v) { auto e = (DrumKick23*)ctx; e->multiFx.setEffect(v); strcpy(e->fxName, e->multiFx.getEffectName()); }, // Skip Format
        .hydrateFn = [](void* ctx, const char* valStr) { auto e = (DrumKick23*)ctx; e->multiFx.setEffect(valStr); } }); // Skip Format
    Param& fxAmt = addParam({ .key = "fxAmt", .label = "FX Amount", .unit = "%", .value = 0.0f });

    DrumKick23(const float sampleRate, float* fxBuffer)
        : EngineBase(Drum, "Kick23", params)
        , multiFx(sampleRate, fxBuffer)
        , sampleRate(sampleRate)
        , sampleRateDiv(1.0f / sampleRate)
    {
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        notePitchMod = std::pow(2.0f, (static_cast<float>(note) - 60.0f) / 12.0f);
        phase1 = 0.0f;
        phase2 = 0.0f;
        phaseVCO2 = 0.0f;
        pitchEnv = 1.0f;
        clickEnv = 1.0f;
        noiseEnv = 1.0f;
        fmEnv = 1.0f;
        ampEnv = 1.0f;
        float durSamples = std::max(1.0f, sampleRate * (duration.value * 0.001f));
        ampStep = 1.0f / durSamples;
        driveFeedback = 0.0f;
    }

    float sampleImpl()
    {
        if (ampEnv <= 0.0f) return multiFx.apply(0.0f, fxAmt.value * 0.01f);
        ;
        float currentAmp = ampEnv;
        ampEnv -= ampStep;

        // // 1. PITCH ENVELOPES
        // float spd = lerp(0.005f, 0.15f, (100.0f - sweepSpd.value) * 0.01f);
        // pitchEnv *= Math::exp(-1.0f / (sampleRate * spd));
        // float pMorph = lerp(pitchEnv, pitchEnv * pitchEnv, sweepShp.value * 0.01f);
        // float rootFreq = subFreq.value + (sweepDep.value * 4.0f * pMorph);
        // fmEnv *= Math::exp(-1.0f / (sampleRate * (fmSnap.value * 0.001f)));

        // 1. PITCH ENVELOPES
        float spd = lerp(0.005f, 0.15f, (100.0f - sweepSpd.value) * 0.01f);
        pitchEnv *= Math::exp(-1.0f / (sampleRate * spd));
        float pMorph = lerp(pitchEnv, pitchEnv * pitchEnv, sweepShp.value * 0.01f);

        // APPLY notePitchMod HERE:
        float baseFreq = subFreq.value * notePitchMod;
        float rootFreq = baseFreq + (sweepDep.value * 4.0f * pMorph);
        fmEnv *= Math::exp(-1.0f / (sampleRate * (fmSnap.value * 0.001f)));

        // 2. FM MODULATOR
        phase2 += (rootFreq * fmRatio.value) * sampleRateDiv;
        if (phase2 > 1.0f) phase2 -= 1.0f;
        float modSig = Math::fastSin(PI_X2 * phase2);

        float foldAmt = fmDirt.value * 0.01f;
        float thresh = 1.0f - (foldAmt * 0.6f);
        if (std::abs(modSig) > thresh) modSig = (modSig > 0 ? thresh : -thresh) - (modSig - (modSig > 0 ? thresh : -thresh));
        modSig *= (1.0f / thresh);

        // 3. VCO 1 (Carrier)
        float fmModulation = (modSig * (fmDepth.value * 0.001f) * fmEnv * 0.2f);
        phase1 += (rootFreq * sampleRateDiv) + fmModulation;
        if (phase1 > 1.0f) phase1 -= 1.0f;

        float s1 = Math::fastSin(PI_X2 * phase1);
        float morph1 = vcoMorph.value * 0.01f;
        if (morph1 > 0.0f) {
            float tri = 2.0f * std::abs(2.0f * (phase1 - std::floor(phase1 + 0.5f))) - 1.0f;
            float sq = (s1 > 0.0f) ? 0.7f : -0.7f;
            if (morph1 < 0.5f) s1 = lerp(s1, tri, morph1 * 2.0f);
            else s1 = lerp(tri, sq, (morph1 - 0.5f) * 2.0f);
        }

        // 4. VCO 2 (Harmonic Layer)
        float s2 = 0.0f;
        if (v2Level.value > 0.0f) {
            // Snap Ratio to Harmonic Series (1, 2, 3, 4...)
            // 1=Root, 2=Octave, 3=Fifth above Octave, 4=Two Octaves, etc.
            float musicalRatio = std::floor(v2Harm.value);
            phaseVCO2 += (rootFreq * musicalRatio) * sampleRateDiv;
            if (phaseVCO2 > 1.0f) phaseVCO2 -= 1.0f;
            s2 = Math::fastSin(PI_X2 * phaseVCO2);

            float m2 = v2Morph.value * 0.01f;
            if (m2 > 0.0f) {
                float t2 = 1.0f - (m2 * 0.7f);
                if (std::abs(s2) > t2) s2 = (s2 > 0 ? t2 : -t2) - (s2 - (s2 > 0 ? t2 : -t2));
                s2 *= (1.0f / t2);
            }
        }

        // Combine Oscillators - VCO2 tracks clickEnv slightly to soften it
        float sig = s1 + (s2 * (v2Level.value * 0.01f) * (0.5f + 0.5f * clickEnv));

        if (subHarm.value > 0.0f) sig += Math::fastSin(PI_X2 * phase1 * 0.5f) * (subHarm.value * 0.01f);

        // 5. TRANSIENTS & DISTORTION
        clickEnv *= Math::exp(-1.0f / (sampleRate * 0.002f));
        // if (clickEnv > 0.01f) sig = 0.0f;
        sig += (Noise::sample() * clickEnv * clickAmt.value);
        // sig += (Noise::sample() * clickEnv * clickAmt.value); // this seems to make no difference
        // sig += (Noise::sample() * clickEnv * clickAmt.value);

        noiseEnv *= Math::exp(-1.0f / (sampleRate * noiseTim.value * 0.001f));
        sig += (Noise::sample() * noiseEnv * noiseAmt.value * 0.04f);

        sig *= (1.0f + hardness.value * 0.1f); // <--- kind of clipping
        if (drive.value > 0.0f) sig = applyDrive(sig, drive.value * 0.01f);
        else sig = applyDriveFeedback(sig, -drive.value * 0.01f, driveFeedback);
        sig = applyBoost(sig, bassBoost.value * 0.01f, bassBoostPrevInput, bassBoostPrevOutput);

        // 6. POST
        float fCoeff = (tone.value * 0.01f) * 0.5f;
        lowPassState += fCoeff * (sig - lowPassState);
        sig = lerp(sig, lowPassState, 0.6f);

        if (compress.value > 0.0f) sig = applyCompression2(sig, compress.value * 0.01f, compressionState);
        sig = multiFx.apply(sig, fxAmt.value * 0.01f);

        return sig * currentAmp * velocity;
    }
};