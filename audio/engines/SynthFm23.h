#pragma once

#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"
#include "helpers/clamp.h"

#include <algorithm>
#include <cmath>
#include <cstring>

// 2-operator FM synth (carrier + modulator).
//
// Carrier envelope  : Attack / hold-at-1 / Release.
//                     Holds full amplitude while gate is open; releases on note-off.
//                     Rates pre-calculated on noteOn for zero per-sample overhead.
//
// Modulator envelope: full ADSR.
//                     All four rates pre-calculated on noteOn.
//                     This is the primary source of timbral movement.
//
// Mod Index response: quadratic curve so the knob feels perceptually even.
// Feedback          : max ≈ π/4, one-pole smoothed to prevent discontinuities.
// Filter            : SVF morph  -100 % LP … 0 bypass … +100 % HP.
// LFO               : → pitch (semitones), → mod-index, → filter cutoff.
// Vel → Index       : additive boost so soft notes still produce sound.

class SynthFm23 : public EngineBase<SynthFm23> {

public:
    static constexpr int DELAY_BUF_SIZE  = 48000;
    static constexpr int REVERB_BUF_SIZE = 16384;

protected:
    const float sampleRate;
    const float sampleRateDiv;

    FilterSVF svfFilter;

    // ── Oscillator state ──────────────────────────────────────────────────────
    float phase    = 0.0f;
    float modPhase = 0.0f;
    float lfoPhase = 0.0f;

    float currentFreq = 440.0f;
    float targetFreq  = 440.0f;

    // ── Voice state ───────────────────────────────────────────────────────────
    float velocity = 1.0f;
    bool  gateOpen = false;

    // ── Carrier envelope  (Attack / hold-at-1 / Release) ─────────────────────
    // Stage: 0=off  1=attack  2=hold  3=release
    float carEnv        = 0.0f;
    int   carStage      = 0;
    float carAttackRate = 0.0f; // pre-calculated on noteOn
    float carReleaseRate = 0.0f; // pre-calculated on noteOn

    // ── Modulator ADSR ────────────────────────────────────────────────────────
    // Stage: 0=off  1=attack  2=decay  3=sustain  4=release
    float modEnv        = 0.0f;
    int   modStage      = 0;
    float modAttackRate = 0.0f; // pre-calculated on noteOn
    float modDecayTau   = 0.0f; // pre-calculated on noteOn
    float modSustainLvl = 0.0f; // pre-calculated on noteOn
    float modReleaseTau = 0.0f; // pre-calculated on noteOn

    // ── Modulator feedback (smoothed) ─────────────────────────────────────────
    float modFbSmooth = 0.0f;

    // ── Delay ─────────────────────────────────────────────────────────────────
    float* delayBuf    = nullptr;
    int    delayWrite  = 0;
    float  dlyFbSmooth = 0.0f;

    // ── Reverb ────────────────────────────────────────────────────────────────
    float* reverbBuf = nullptr;

    static constexpr int COMB_LEN[4] = { 1559, 1617, 1685, 1751 };
    static constexpr int AP_LEN[3]   = { 347, 113, 37 };

    int   combOff[4] = {};
    int   apOff[3]   = {};
    int   combIdx[8] = {};
    int   apIdx[4]   = {};
    float combFb[8]  = {};

    // ── Helpers ───────────────────────────────────────────────────────────────
    static float lerp(float a, float b, float t) { return a + t * (b - a); }

    float tau(float ms) const
    {
        return (ms < 0.01f) ? 0.0f : Math::exp(-1.0f / (sampleRate * ms * 0.001f));
    }

    float linearRate(float ms) const
    {
        if (ms < 0.01f) return 1.0f;
        return 1.0f / (sampleRate * ms * 0.001f);
    }

    // ── Carrier envelope tick  (uses pre-calculated rates) ───────────────────
    float carEnvTick()
    {
        switch (carStage) {
        case 0:
            carEnv = 0.0f;
            break;
        case 1: // attack
            carEnv += carAttackRate;
            if (carEnv >= 1.0f) {
                carEnv   = 1.0f;
                carStage = gateOpen ? 2 : 3;
            }
            break;
        case 2: // hold at 1.0 while gate is open
            carEnv = 1.0f;
            if (!gateOpen) carStage = 3;
            break;
        case 3: // release
            // carEnv *= carReleaseTau;
            carEnv -= carReleaseRate;
            if (carEnv < 0.0001f) { carEnv = 0.0f; carStage = 0; }
            break;
        }
        return carEnv;
    }

    // ── Modulator ADSR tick  (uses pre-calculated rates) ─────────────────────
    float modEnvTick()
    {
        switch (modStage) {
        case 0:
            modEnv = 0.0f;
            break;
        case 1: // attack
            modEnv += modAttackRate;
            if (modEnv >= 1.0f) { modEnv = 1.0f; modStage = 2; }
            break;
        case 2: // decay toward sustain
            modEnv = modSustainLvl + (modEnv - modSustainLvl) * modDecayTau;
            if (modEnv <= modSustainLvl + 0.0001f) { modEnv = modSustainLvl; modStage = 3; }
            break;
        case 3: // sustain hold
            modEnv = modSustainLvl;
            if (!gateOpen) modStage = 4;
            break;
        case 4: // release
            modEnv *= modReleaseTau;
            if (modEnv < 0.0001f) { modEnv = 0.0f; modStage = 0; }
            break;
        }
        return modEnv;
    }

    // ── Exponential mod-index curve ───────────────────────────────────────────
    // Maps knob value 0..20 to a perceptually even response.
    // At index=0  → 0,  index=5  → ~1.25,  index=10 → ~5,  index=20 → ~20
    static float indexCurve(float x)
    {
        // x²/20  gives a gentle quadratic that preserves the 0..20 range end-point
        return (x * x) * 0.05f;
    }

    // ── LP / bypass / HP morphing filter ─────────────────────────────────────
    float applyMorphFilter(float sig, float cutoffParam, float res)
    {
        float absC = std::abs(cutoffParam);
        if (absC < 0.5f) return sig;

        float normCutoff = 0.01f + absC * 0.0098f;
        float normRes    = CLAMP(res, 0.0f, 0.98f);

        svfFilter.setCutoff(normCutoff);
        svfFilter.setResonance(normRes);
        auto out = svfFilter.process12(sig);

        float t        = CLAMP((cutoffParam + 100.0f) * 0.005f, 0.0f, 1.0f);
        float filtered = lerp(out.lp, out.hp, t);
        return lerp(sig, filtered, absC * 0.01f);
    }

    // ── FX ────────────────────────────────────────────────────────────────────
    float reverbProcess(float in, float mix, float size, float damp)
    {
        if (mix < 0.001f) return in;

        float decay = 0.7f + size * 0.28f;
        float d     = 0.2f + damp * 0.7f;
        float invD  = 1.0f - d;
        float wet   = 0.0f;

        for (int c = 0; c < 4; ++c) {
            float* buf = &reverbBuf[combOff[c]];
            int    idx = combIdx[c];
            float  del = buf[idx];
            combFb[c]  = del * invD + combFb[c] * d;
            buf[idx]   = in + combFb[c] * decay;
            if (++idx >= COMB_LEN[c]) idx = 0;
            combIdx[c] = idx;
            wet += del;
        }
        wet *= 0.25f;

        for (int a = 0; a < 3; ++a) {
            float* buf = &reverbBuf[apOff[a]];
            int    idx = apIdx[a];
            float  del = buf[idx];
            float  v   = wet + del * 0.5f;
            buf[idx]   = v;
            wet        = del - v * 0.5f;
            if (++idx >= AP_LEN[a]) idx = 0;
            apIdx[a] = idx;
        }

        return in + wet * mix;
    }

    float delayProcess(float sig)
    {
        if (dlyMix.value < 0.001f) return sig;

        int   delaySmp = std::max(1, std::min((int)(dlyTime.value * 0.001f * sampleRate), DELAY_BUF_SIZE - 1));
        int   readIdx  = (delayWrite - delaySmp + DELAY_BUF_SIZE) % DELAY_BUF_SIZE;
        float delayed  = delayBuf[readIdx];

        float fbTarget  = dlyFdbk.value * 0.01f * 0.85f;
        dlyFbSmooth    += 0.001f * (fbTarget - dlyFbSmooth);

        delayBuf[delayWrite] = sig + delayed * dlyFbSmooth;
        delayWrite = (delayWrite + 1) % DELAY_BUF_SIZE;

        return lerp(sig, sig + delayed * 0.7f, dlyMix.value * 0.01f);
    }

    float bufferedFxProcess(float sig)
    {
        sig = delayProcess(sig);
        sig = reverbProcess(sig, reverbMix.value * 0.01f, reverbSize.value * 0.01f, reverbDamp.value * 0.01f);
        return sig;
    }

public:
    Param params[21] = {
        { .label = "Frequency",  .unit = "Hz",  .value = 440.0f,  .min = 20.0f,   .max = 2000.0f, .step = 0.5f  }, // 0
        { .label = "Ratio",      .unit = "x",   .value = 2.0f,    .min = 0.25f,   .max = 16.0f,   .step = 0.01f }, // 1
        { .label = "Mod Attack", .unit = "ms",   .value = 5.0f,    .min = 0.5f,    .max = 2000.0f, .step = 1.0f  }, // 3
        { .label = "Mod Decay",  .unit = "ms",   .value = 300.0f,  .min = 5.0f,    .max = 4000.0f, .step = 5.0f  }, // 4
        { .label = "Mod Sust",   .unit = "%",    .value = 20.0f,   .min = 0.0f,    .max = 100.0f               }, // 5
        { .label = "Mod Rel",    .unit = "ms",   .value = 200.0f,  .min = 5.0f,    .max = 4000.0f, .step = 5.0f  }, // 6
        { .label = "Feedback",   .unit = "%",    .value = 0.0f,    .min = 0.0f,    .max = 100.0f               }, // 7
        { .label = "Car Attack", .unit = "ms",   .value = 10.0f,   .min = 1.0f,    .max = 2000.0f, .step = 1.0f  }, // 9
        { .label = "Car Rel",    .unit = "ms",   .value = 300.0f,  .min = 5.0f,    .max = 4000.0f, .step = 5.0f  }, // 10
        { .label = "LFO Rate",   .unit = "Hz",   .value = 2.0f,    .min = 0.05f,   .max = 30.0f,   .step = 0.05f }, // 11
        { .label = "LFO Pitch",  .unit = "st",   .value = 0.0f,    .min = 0.0f,    .max = 12.0f,   .step = 0.1f  }, // 12
        { .label = "LFO Index",  .unit = "%",    .value = 0.0f,    .min = 0.0f,    .max = 100.0f               }, // 13
        { .label = "LFO Cutoff", .unit = "%",    .value = 0.0f,    .min = 0.0f,    .max = 100.0f               }, // 14
        { .label = "Cutoff",     .unit = "%",    .value = 0.0f,    .min = -100.0f, .max = 100.0f               }, // 15
        { .label = "Resonance",  .unit = "%",    .value = 20.0f,   .min = 0.0f,    .max = 100.0f               }, // 16
        { .label = "Reverb Mix", .unit = "%",    .value = 0.0f,    .min = 0.0f,    .max = 100.0f               }, // 17
        { .label = "Rvb Size",   .unit = "%",    .value = 50.0f,   .min = 0.0f,    .max = 100.0f               }, // 18
        { .label = "Rvb Damp",   .unit = "%",    .value = 50.0f,   .min = 0.0f,    .max = 100.0f               }, // 19
        { .label = "Dly Mix",    .unit = "%",    .value = 0.0f,    .min = 0.0f,    .max = 100.0f               }, // 20
        { .label = "Dly Time",   .unit = "ms",   .value = 125.0f,  .min = 10.0f,   .max = 1000.0f, .step = 5.0f  }, // 21
        { .label = "Dly Fdbk",   .unit = "%",    .value = 0.0f,    .min = 0.0f,    .max = 100.0f               }, // 22
    };

    Param& freq        = params[0];
    Param& ratio       = params[1];
    Param& modAttack   = params[2];
    Param& modDecay    = params[3];
    Param& modSustain  = params[4];
    Param& modRelease  = params[5];
    Param& feedback    = params[6];
    Param& carAttack   = params[7];
    Param& carRelease  = params[8];
    Param& lfoRate     = params[9];
    Param& lfoToPitch  = params[10];
    Param& lfoToIndex  = params[11];
    Param& lfoToCutoff = params[12];
    Param& cutoff      = params[13];
    Param& resonance   = params[14];
    Param& reverbMix   = params[15];
    Param& reverbSize  = params[16];
    Param& reverbDamp  = params[17];
    Param& dlyMix      = params[18];
    Param& dlyTime     = params[19];
    Param& dlyFdbk     = params[20];

    SynthFm23(float sr, float* dlBuf, float* rvBuf)
        : EngineBase(Synth, "Fm23", params)
        , sampleRate(sr)
        , sampleRateDiv(1.0f / sr)
        , delayBuf(dlBuf)
        , reverbBuf(rvBuf)
    {
        if (delayBuf)
            for (int i = 0; i < DELAY_BUF_SIZE; ++i) delayBuf[i] = 0.0f;

        if (reverbBuf) {
            int pos = 0;
            for (int c = 0; c < 4; ++c) { combOff[c] = pos; pos += COMB_LEN[c]; }
            for (int a = 0; a < 3; ++a) { apOff[a]   = pos; pos += AP_LEN[a];   }
            for (int i = 0; i < REVERB_BUF_SIZE; ++i) reverbBuf[i] = 0.0f;
        }

        init();
    }

    void noteOnImpl(uint8_t note, float vel)
    {
        velocity = vel;

        float noteOffset = static_cast<float>(note) - 60.0f;
        targetFreq  = freq.value * std::pow(2.0f, noteOffset / 12.0f);
        currentFreq = targetFreq;

        gateOpen = true;

        // ── Pre-calculate carrier rates ──────────────────────────────────────
        carAttackRate = linearRate(carAttack.value);
        carReleaseRate = linearRate(carRelease.value);
        carStage      = 1;

        // ── Pre-calculate modulator ADSR rates ───────────────────────────────
        modAttackRate = linearRate(modAttack.value);
        modDecayTau   = tau(modDecay.value);
        modSustainLvl = modSustain.value * 0.01f;
        modReleaseTau = tau(modRelease.value);
        modEnv        = 0.0f;
        modStage      = 1;
    }

    void noteOffImpl(uint8_t)
    {
        gateOpen = false;
        // carEnvTick() transitions to release via hold→release check
        // modEnvTick() transitions to release via sustain→release check
    }

    float sampleImpl()
    {
        if (carStage == 0 && modStage == 0 && !gateOpen)
            return bufferedFxProcess(0.0f);

        // ── 1. LFO ────────────────────────────────────────────────────────────
        lfoPhase += lfoRate.value * sampleRateDiv;
        if (lfoPhase > 1.0f) lfoPhase -= 1.0f;
        float lfoOut = Math::fastSin(PI_X2 * lfoPhase); // -1..+1

        // ── 2. ENVELOPES ──────────────────────────────────────────────────────
        float carLvl = carEnvTick();
        float modLvl = modEnvTick();

        // ── 3. MODULATOR ──────────────────────────────────────────────────────
        // Pitch LFO: semitones → frequency ratio, clamped > 0
        float pitchRatio  = std::pow(2.0f, lfoOut * lfoToPitch.value / 12.0f);
        float carrierFreq = std::max(1.0f, currentFreq * pitchRatio);
        float modulatorFreq = carrierFreq * ratio.value;

        // Feedback: use smoothed previous modulator output to avoid discontinuities.
        // Max phase offset ≈ π/4 at 100 % — audible harmonic colour without chaos.
        float fbPhase = modFbSmooth * (feedback.value * 0.01f) * (float)(M_PI * 0.25);

        modPhase += modulatorFreq * sampleRateDiv;
        if (modPhase > 1.0f) modPhase -= 1.0f;

        float rawMod  = Math::fastSin(PI_X2 * modPhase + fbPhase);
        // Smooth toward current output for next sample
        modFbSmooth  += 0.3f * (rawMod - modFbSmooth);

        float lfoScale   = 1.0f + lfoOut * lfoToIndex.value * 0.01f;
        float effectiveIndex = modLvl * lfoScale;

        // ── 4. CARRIER ────────────────────────────────────────────────────────
        phase += carrierFreq * sampleRateDiv;
        if (phase > 1.0f) phase -= 1.0f;

        float sig = Math::fastSin(PI_X2 * phase + PI_X2 * effectiveIndex * rawMod);

        // ── 5. AMP ENVELOPE ───────────────────────────────────────────────────
        sig *= carLvl * velocity;

        // ── 6. FILTER ─────────────────────────────────────────────────────────
        sig = CLAMP(sig, -1.0f, 1.0f);
        float dynCutoff = CLAMP(cutoff.value + lfoOut * lfoToCutoff.value, -100.0f, 100.0f);
        sig = applyMorphFilter(sig, dynCutoff, resonance.value * 0.01f);

        // ── 7. BUFFERED FX ────────────────────────────────────────────────────
        sig = bufferedFxProcess(sig);

        return sig;
    }
};