#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/MultiFx.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"
#include <cstring>
#include <algorithm>

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
    float lowPassState = 0.0f;
    float driveFeedback = 0.0f;
    char fxName[24] = "Off";

    float lerp(float a, float b, float t) { return a + t * (b - a); }

public:
    Param params[24] = {
        // --- PAGE 1 ---
        { .label = "Duration", .unit = "ms", .value = 500.0f, .min = 50.0f, .max = 2500.0f }, // 0
        { .label = "Amp Env", .unit = "%", .value = 20.0f, .onUpdate = [](void* ctx, float val) { ((DrumKick23*)ctx)->envelopAmp.morph(val * 0.01f); } }, // 1
        { .label = "Sub Freq", .unit = "Hz", .value = 45.0f, .min = 30.0f, .max = 100.0f }, // 2
        { .label = "Sweep Dep", .unit = "%", .value = 60.0f }, // 3
        { .label = "Sweep Spd", .unit = "%", .value = 30.0f }, // 4 (REPLACED Decay Slp)
        { .label = "VCO Morph", .unit = "Tri-Sq", .value = 0.0f }, // 5
        { .label = "Symmetry", .unit = "%", .value = 0.0f }, // 6
        { .label = "Hardness", .unit = "%", .value = 30.0f }, // 7
        { .label = "Click Amt", .unit = "%", .value = 20.0f }, // 8
        { .label = "Noise Amt", .unit = "%", .value = 10.0f }, // 9
        { .label = "Noise Tim", .unit = "ms", .value = 20.0f, .min = 2.0f, .max = 200.0f }, // 10
        { .label = "Sweep Shp", .unit = "%", .value = 50.0f }, // 11

        // --- PAGE 2 ---
        { .label = "FM Dirt", .unit = "%", .value = 0.0f }, // 12
        { .label = "FM Ratio", .unit = "mult", .value = 1.618f, .min = 0.5f, .max = 8.0f, .step = 0.01f }, // 13
        { .label = "FM Depth", .unit = "%", .value = 0.0f }, // 14
        { .label = "Sub FM", .unit = "%", .value = 0.0f }, // 15
        { .label = "Sub Harm", .unit = "%", .value = 0.0f }, // 16
        { .label = "Drive", .unit = "%", .value = 20.0f }, // 17
        { .label = "Drive Shp", .unit = "%", .value = 50.0f }, // 18
        { .label = "Compress", .unit = "%", .value = 10.0f }, // 19
        { .label = "Tone", .unit = "%", .value = 100.0f }, // 20
        { .label = "Velo Sens", .unit = "%", .value = 50.0f }, // 21
        { .label = "FX Type", .string = fxName, .value = 0.0f, .max = (float)MultiFx::FX_COUNT - 1, .step = 1.0f, .onUpdate = [](void* ctx, float v) { 
            auto e = (DrumKick23*)ctx; e->multiFx.setEffect(v); strcpy(e->fxName, e->multiFx.getEffectName()); } }, // 22
        { .label = "FX Amount", .unit = "%", .value = 0.0f } // 23
    };

    // --- References ---
    Param& duration = params[0];   Param& ampEnv   = params[1];    Param& subFreq  = params[2];
    Param& sweepDep = params[3];   Param& sweepSpd = params[4];    Param& vcoMorph = params[5];
    Param& symmetry = params[6];   Param& hardness = params[7];    Param& clickAmt = params[8];
    Param& noiseAmt = params[9];   Param& noiseTim = params[10];   Param& sweepShp = params[11];
    Param& fmDirt   = params[12];  Param& fmRatio  = params[13];   Param& fmDepth  = params[14];
    Param& subFm    = params[15];  Param& subHarm  = params[16];   Param& drive    = params[17];
    Param& driveShp = params[18];  Param& compress = params[19];   Param& tone     = params[20];
    Param& veloSens = params[21];  Param& fxType   = params[22];   Param& fxAmt    = params[23];

    DrumKick23(const float sampleRate, float* fxBuffer)
        : EngineBase(Drum, "Kick23", params), multiFx(sampleRate, fxBuffer), sampleRate(sampleRate), sampleRateDiv(1.0f / sampleRate) { init(); }

    void noteOnImpl(uint8_t note, float _velocity) {
        velocity = _velocity;
        phase1 = 0.0f; phase2 = 0.0f;
        pitchEnv = 1.0f; clickEnv = 1.0f; noiseEnv = 1.0f;
        driveFeedback = 0.0f;
        envelopAmp.reset(static_cast<int>(sampleRate * (duration.value * 0.001f)));
    }

    float sampleImpl() {
        float amp = envelopAmp.next();
        if (amp < 0.0001f) return 0.0f;

        float vMod = lerp(1.0f - (veloSens.value * 0.01f), velocity, veloSens.value * 0.01f);

        // 1. PITCH ENV - Now with Speed control
        // High speed = shorter decay time (sharper snap)
        float spd = lerp(0.005f, 0.15f, (100.0f - sweepSpd.value) * 0.01f);
        pitchEnv *= Math::exp(-1.0f / (sampleRate * spd));
        
        // Shape control: blend between linear and exponential pitch drop
        float pMorph = lerp(pitchEnv, pitchEnv * pitchEnv, sweepShp.value * 0.01f);
        float rootFreq = subFreq.value + (sweepDep.value * 4.0f * pMorph);

        // 2. FM STAGE
        phase2 += (rootFreq * fmRatio.value) * sampleRateDiv;
        if (phase2 > 1.0f) phase2 -= 1.0f;
        
        float modSig = Math::fastSin(PI_X2 * phase2);
        if (fmDirt.value > 0.0f) {
            float dirt = fmDirt.value * 0.01f;
            modSig = lerp(modSig, (modSig > 0 ? 1.0f : -1.0f), dirt);
        }

        float totalFm = (fmDepth.value * 0.05f * pMorph * vMod) + (subFm.value * 0.02f);

        // 3. OSCILLATOR
        phase1 += (rootFreq * sampleRateDiv) + (modSig * totalFm);
        if (phase1 > 1.0f) phase1 -= 1.0f;

        float s = Math::fastSin(PI_X2 * phase1);
        
        // Morph logic
        float morph = vcoMorph.value * 0.01f;
        if (morph > 0.0f) {
            float sq = (s > 0 ? 1.0f : -1.0f);
            float tri = 4.0f * std::abs(phase1 - std::floor(phase1 + 0.5f)) - 1.0f;
            s = lerp(s, lerp(tri, sq, 0.5f), morph);
        }
        
        s += (symmetry.value * 0.01f) * (1.0f - std::abs(s));
        if (subHarm.value > 0.0f) s += Math::fastSin(PI_X2 * phase1 * 0.5f) * (subHarm.value * 0.01f);

        // 4. TRANSIENTS
        clickEnv *= Math::exp(-1.0f / (sampleRate * 0.002f));
        noiseEnv *= Math::exp(-1.0f / (sampleRate * noiseTim.value * 0.001f));
        float transient = (Noise::sample() * clickEnv * clickAmt.value * 0.12f) + (Noise::sample() * noiseEnv * noiseAmt.value * 0.04f);
        float sig = s + transient;

        // 5. HARDNESS (Aggressive Clipping)
        float hrd = 1.0f + (hardness.value * 0.15f * vMod);
        sig = std::max(-1.0f, std::min(1.0f, sig * hrd));

        // 6. DRIVE & SHAPE (The "Growl")
        float drvAmt = drive.value * 0.01f * 6.0f * vMod;
        float shp = driveShp.value * 0.01f;
        
        float feedback = driveFeedback * shp * 0.9f;
        sig = applyDrive(sig + feedback, drvAmt);
        driveFeedback = sig; 

        // 7. POST (Tone Filter)
        float fCoeff = (tone.value * 0.01f) * 0.5f;
        lowPassState += fCoeff * (sig - lowPassState);
        sig = lerp(sig, lowPassState, 0.6f);

        if (compress.value > 0.0f) sig = applyCompression(sig, compress.value * 0.01f);
        sig = multiFx.apply(sig, fxAmt.value * 0.01f);

        return sig * amp * vMod;
    }
};