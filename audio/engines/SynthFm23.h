#pragma once

#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"
#include "helpers/clamp.h"

#include <algorithm>
#include <cmath>
#include <cstring>

// 3-operator FM synth (carrier + Op2 + Op3).
//
// Carrier envelope  : Attack / hold-at-1 / Release.
// Op2 / Op3         : ASR envelope. Sustain = held mod depth (0..1).
//                     Attack ramps 0→1, Release ramps back to 0.
//                     Envelope * INDEX_SCALE gives the FM index.
//
// Algorithm         :
//   1 — Serial:     Op3 → Op2 → Carrier
//   2 — Parallel:   Op2 → Carrier,  Op3 → Carrier
//   3 — Branch:     Op3 → Op2,  Op3 → Carrier
//   4 — Additive:   Op2 → Carrier,  Op3 = free carrier

class SynthFm23 : public EngineBase<SynthFm23> {

public:
    static constexpr int DELAY_BUF_SIZE = 48000;
    static constexpr int REVERB_BUF_SIZE = 16384;
    static constexpr float INDEX_SCALE = 4.0f;

protected:
    const float sampleRate;
    const float sampleRateDiv;

    FilterSVF svfFilter;

    // ── Oscillator state ──────────────────────────────────────────────────────
    float phase = 0.0f;
    float modPhase = 0.0f;
    float mod2Phase = 0.0f;
    float lfoPhase = 0.0f;

    float currentFreq = 440.0f;
    float targetFreq = 440.0f;

    // ── Voice state ───────────────────────────────────────────────────────────
    float velocity = 1.0f;
    bool gateOpen = false;

    // ── Carrier envelope (Attack / hold / Release) ────────────────────────────
    float carEnv = 0.0f;
    int carStage = 0;
    float carAttackRate = 0.0f;
    float carReleaseRate = 0.0f;

    // ── Op2 ASR ───────────────────────────────────────────────────────────────
    // Stage: 0=off  1=attack  2=sustain  3=release
    float modEnv = 0.0f;
    int modStage = 0;
    float modAttackRate = 0.0f;
    float modSustainLvl = 0.0f;
    float modReleaseRate = 0.0f;

    // ── Op3 ASR ───────────────────────────────────────────────────────────────
    float mod2Env = 0.0f;
    int mod2Stage = 0;
    float mod2AttackRate = 0.0f;
    float mod2SustainLvl = 0.0f;
    float mod2ReleaseRate = 0.0f;

    // ── Op2 feedback (smoothed) ───────────────────────────────────────────────
    float modFbSmooth = 0.0f;

    // ── Delay ─────────────────────────────────────────────────────────────────
    float* delayBuf = nullptr;
    int delayWrite = 0;
    float dlyFbSmooth = 0.0f;

    // ── Reverb ────────────────────────────────────────────────────────────────
    float* reverbBuf = nullptr;

    static constexpr int COMB_LEN[4] = { 1559, 1617, 1685, 1751 };
    static constexpr int AP_LEN[3] = { 347, 113, 37 };

    int combOff[4] = {};
    int apOff[3] = {};
    int combIdx[8] = {};
    int apIdx[4] = {};
    float combFb[8] = {};

    // ── Helpers ───────────────────────────────────────────────────────────────
    static float lerp(float a, float b, float t) { return a + t * (b - a); }

    float linearRate(float ms) const
    {
        if (ms < 0.01f) return 1.0f;
        return 1.0f / (sampleRate * ms * 0.001f);
    }

    // ── Carrier envelope tick (identical to working 2-op) ────────────────────
    float carEnvTick()
    {
        switch (carStage) {
        case 0:
            carEnv = 0.0f;
            break;
        case 1: // attack
            carEnv += carAttackRate;
            if (carEnv >= 1.0f) {
                carEnv = 1.0f;
                carStage = gateOpen ? 2 : 3;
            }
            break;
        case 2: // hold at 1.0 while gate open
            carEnv = 1.0f;
            if (!gateOpen) carStage = 3;
            break;
        case 3: // release
            carEnv -= carReleaseRate;
            if (carEnv < 0.0001f) {
                carEnv = 0.0f;
                carStage = 0;
            }
            break;
        }
        return carEnv;
    }

    // ── Op2 ASR tick ──────────────────────────────────────────────────────────
    float modEnvTick()
    {
        switch (modStage) {
        case 0:
            modEnv = 0.0f;
            break;
        case 1: // attack → sustain level
            modEnv += modAttackRate;
            if (modEnv >= 1.0f) {
                modEnv = 1.0f;
                modStage = 2;
            }
            break;
        case 2: // hold at 1.0, then drop to sustain... actually just hold at 1.0
            // Sustain level is applied as a multiplier at the call site,
            // so the envelope itself holds at 1.0 during the gate.
            modEnv = 1.0f;
            if (!gateOpen) modStage = 3;
            break;
        case 3: // release
            modEnv -= modReleaseRate;
            if (modEnv < 0.0001f) {
                modEnv = 0.0f;
                modStage = 0;
            }
            break;
        }
        return modEnv;
    }

    // ── Op3 ASR tick ──────────────────────────────────────────────────────────
    float mod2EnvTick()
    {
        switch (mod2Stage) {
        case 0:
            mod2Env = 0.0f;
            break;
        case 1: // attack
            mod2Env += mod2AttackRate;
            if (mod2Env >= 1.0f) {
                mod2Env = 1.0f;
                mod2Stage = 2;
            }
            break;
        case 2: // sustain hold
            mod2Env = 1.0f;
            if (!gateOpen) mod2Stage = 3;
            break;
        case 3: // release
            mod2Env -= mod2ReleaseRate;
            if (mod2Env < 0.0001f) {
                mod2Env = 0.0f;
                mod2Stage = 0;
            }
            break;
        }
        return mod2Env;
    }

    // ── LP / bypass / HP morphing filter ─────────────────────────────────────
    float applyMorphFilter(float sig, float cutoffParam, float res)
    {
        float absC = std::abs(cutoffParam);
        if (absC < 0.5f) return sig;

        float normCutoff = 0.01f + absC * 0.0098f;
        float normRes = CLAMP(res, 0.0f, 0.98f);

        svfFilter.setCutoff(normCutoff);
        svfFilter.setResonance(normRes);
        auto out = svfFilter.process12(sig);

        float t = CLAMP((cutoffParam + 100.0f) * 0.005f, 0.0f, 1.0f);
        float filtered = lerp(out.lp, out.hp, t);
        return lerp(sig, filtered, absC * 0.01f);
    }

    // ── FX ────────────────────────────────────────────────────────────────────
    float reverbProcess(float in, float mix, float size, float damp)
    {
        if (mix < 0.001f) return in;

        float decay = 0.7f + size * 0.28f;
        float d = 0.2f + damp * 0.7f;
        float invD = 1.0f - d;
        float wet = 0.0f;

        for (int c = 0; c < 4; ++c) {
            float* buf = &reverbBuf[combOff[c]];
            int idx = combIdx[c];
            float del = buf[idx];
            combFb[c] = del * invD + combFb[c] * d;
            buf[idx] = in + combFb[c] * decay;
            if (++idx >= COMB_LEN[c]) idx = 0;
            combIdx[c] = idx;
            wet += del;
        }
        wet *= 0.25f;

        for (int a = 0; a < 3; ++a) {
            float* buf = &reverbBuf[apOff[a]];
            int idx = apIdx[a];
            float del = buf[idx];
            float v = wet + del * 0.5f;
            buf[idx] = v;
            wet = del - v * 0.5f;
            if (++idx >= AP_LEN[a]) idx = 0;
            apIdx[a] = idx;
        }

        return in + wet * mix;
    }

    float delayProcess(float sig)
    {
        if (dlyMix.value < 0.001f) return sig;

        int delaySmp = std::max(1, std::min((int)(dlyTime.value * 0.001f * sampleRate), DELAY_BUF_SIZE - 1));
        int readIdx = (delayWrite - delaySmp + DELAY_BUF_SIZE) % DELAY_BUF_SIZE;
        float delayed = delayBuf[readIdx];

        float fbTarget = dlyFdbk.value * 0.01f * 0.85f;
        dlyFbSmooth += 0.001f * (fbTarget - dlyFbSmooth);

        delayBuf[delayWrite] = sig + delayed * dlyFbSmooth;
        delayWrite = (delayWrite + 1) % DELAY_BUF_SIZE;

        return lerp(sig, sig + delayed * 0.7f, dlyMix.value * 0.01f);
    }

    float bufferedFxProcess(float sig)
    {
        sig = delayProcess(sig);
        sig = reverbProcess(sig, reverbMix.value * 0.01f, 0.5f, reverbDamp.value * 0.01f);
        return sig;
    }

public:
    // NOTE: param count matches original (18) + 6 new = 24.
    // Layout kept identical to the working 2-op for params 0-17,
    // new params appended at 18-23 to avoid any index shifting.
    Param params[24] = {
        { .label = "Attack", .unit = "ms", .value = 10.0f, .min = 1.0f, .max = 2000.0f, .step = 1.0f }, // 0
        { .label = "Release", .unit = "ms", .value = 300.0f, .min = 5.0f, .max = 4000.0f, .step = 5.0f }, // 1
        { .label = "Frequency", .unit = "Hz", .value = 440.0f, .min = 20.0f, .max = 2000.0f, .step = 0.5f }, // 2
        { .label = "Ratio 2", .unit = "x", .value = 2.0f, .min = 0.25f, .max = 16.0f, .step = 0.01f }, // 3
        { .label = "Op2 Depth", .unit = "%", .value = 80.0f, .min = 0.0f, .max = 100.0f }, // 5  sustain = held mod depth
        { .label = "Op2 Attack", .unit = "ms", .value = 5.0f, .min = 0.5f, .max = 2000.0f, .step = 1.0f }, // 4
        { .label = "Op2 Release", .unit = "ms", .value = 200.0f, .min = 5.0f, .max = 4000.0f, .step = 5.0f }, // 6
        { .label = "Ratio 3", .unit = "x", .value = 3.0f, .min = 0.25f, .max = 16.0f, .step = 0.01f }, // 7
        { .label = "Op3 Depth", .unit = "%", .value = 60.0f, .min = 0.0f, .max = 100.0f }, // 9  sustain = held mod depth
        { .label = "Op3 Attack", .unit = "ms", .value = 5.0f, .min = 0.5f, .max = 2000.0f, .step = 1.0f }, // 8
        { .label = "Op3 Release", .unit = "ms", .value = 200.0f, .min = 5.0f, .max = 4000.0f, .step = 5.0f }, // 10
        { .label = "Algorithm", .unit = "", .value = 1.0f, .min = 1.0f, .max = 4.0f, .step = 1.0f }, // 11
        { .label = "LFO Rate", .unit = "Hz", .value = 2.0f, .min = 0.05f, .max = 30.0f, .step = 0.05f }, // 12
        { .label = "LFO Pitch", .unit = "st", .value = 0.0f, .min = 0.0f, .max = 12.0f, .step = 0.1f }, // 13
        { .label = "LFO Cutoff", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f }, // 14
        { .label = "Cutoff", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f }, // 15
        { .label = "Resonance", .unit = "%", .value = 20.0f, .min = 0.0f, .max = 100.0f }, // 16
        { .label = "Reverb Mix", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f }, // 17
        { .label = "Rvb Damp", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f }, // 18
        { .label = "Dly Mix", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f }, // 19
        { .label = "Dly Time", .unit = "ms", .value = 125.0f, .min = 10.0f, .max = 1000.0f, .step = 5.0f }, // 20
        { .label = "Dly Fdbk", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f }, // 21
        // 2 spare slots
        { .label = "Spare 22", .unit = "", .value = 0.0f, .min = 0.0f, .max = 100.0f }, // 22
        { .label = "Spare 23", .unit = "", .value = 0.0f, .min = 0.0f, .max = 100.0f }, // 23
    };

    Param& carAttack = params[0];
    Param& carRelease = params[1];
    Param& freq = params[2];
    Param& ratio = params[3];
    Param& modDepth = params[4]; // Op2 sustained mod depth (0-100%)
    Param& modAttack = params[5];
    Param& modRelease = params[6];
    Param& ratio2 = params[7];
    Param& op3Depth = params[8]; // Op3 sustained mod depth (0-100%)
    Param& op3Attack = params[9];
    Param& op3Release = params[10];
    Param& algorithm = params[11];
    Param& lfoRate = params[12];
    Param& lfoToPitch = params[13];
    Param& lfoToCutoff = params[14];
    Param& cutoff = params[15];
    Param& resonance = params[16];
    Param& reverbMix = params[17];
    Param& reverbDamp = params[18];
    Param& dlyMix = params[19];
    Param& dlyTime = params[20];
    Param& dlyFdbk = params[21];

    SynthFm23(float sr, float* dlBuf, float* rvBuf)
        : EngineBase(Synth, "Fm23", params)
        , sampleRate(sr)
        , sampleRateDiv(1.0f / sr)
        , delayBuf(dlBuf)
        , reverbBuf(rvBuf)
    {
        if (delayBuf)
            for (int i = 0; i < DELAY_BUF_SIZE; ++i)
                delayBuf[i] = 0.0f;

        if (reverbBuf) {
            int pos = 0;
            for (int c = 0; c < 4; ++c) {
                combOff[c] = pos;
                pos += COMB_LEN[c];
            }
            for (int a = 0; a < 3; ++a) {
                apOff[a] = pos;
                pos += AP_LEN[a];
            }
            for (int i = 0; i < REVERB_BUF_SIZE; ++i)
                reverbBuf[i] = 0.0f;
        }

        init();
    }

    void noteOnImpl(uint8_t note, float vel)
    {
        velocity = vel;

        float noteOffset = static_cast<float>(note) - 60.0f;
        targetFreq = freq.value * std::pow(2.0f, noteOffset / 12.0f);
        currentFreq = targetFreq;

        gateOpen = true;

        // ── Carrier (identical to working 2-op) ───────────────────────────────
        carAttackRate = linearRate(carAttack.value);
        carReleaseRate = linearRate(carRelease.value);
        carStage = 1;

        // ── Op2 ASR ───────────────────────────────────────────────────────────
        modAttackRate = linearRate(modAttack.value);
        modSustainLvl = modDepth.value * 0.01f; // 0..1, applied at call site
        modReleaseRate = linearRate(modRelease.value);
        modEnv = 0.0f;
        modStage = 1;

        // ── Op3 ASR ───────────────────────────────────────────────────────────
        mod2AttackRate = linearRate(op3Attack.value);
        mod2SustainLvl = op3Depth.value * 0.01f;
        mod2ReleaseRate = linearRate(op3Release.value);
        mod2Env = 0.0f;
        mod2Stage = 1;
    }

    void noteOffImpl(uint8_t)
    {
        gateOpen = false;
    }

    float sampleImpl()
    {
        // Identical gate check to working 2-op, extended for mod2Stage
        if (carStage == 0 && modStage == 0 && mod2Stage == 0 && !gateOpen)
            return bufferedFxProcess(0.0f);

        // ── 1. LFO ────────────────────────────────────────────────────────────
        lfoPhase += lfoRate.value * sampleRateDiv;
        if (lfoPhase > 1.0f) lfoPhase -= 1.0f;
        float lfoOut = Math::fastSin(PI_X2 * lfoPhase);

        // ── 2. ENVELOPES ──────────────────────────────────────────────────────
        float carLvl = carEnvTick();

        // ASR envelope holds at 1.0; sustain depth applied as multiplier here
        float modLvl = modEnvTick() * modSustainLvl * INDEX_SCALE;
        float mod2Lvl = mod2EnvTick() * mod2SustainLvl * INDEX_SCALE;

        // ── 3. FREQUENCIES ────────────────────────────────────────────────────
        float pitchRatio = std::pow(2.0f, lfoOut * lfoToPitch.value / 12.0f);
        float carrierFreq = std::max(1.0f, currentFreq * pitchRatio);
        float op2Freq = carrierFreq * ratio.value;
        float op3Freq = carrierFreq * ratio2.value;

        // ── 4. OP3 oscillator ─────────────────────────────────────────────────
        mod2Phase += op3Freq * sampleRateDiv;
        if (mod2Phase > 1.0f) mod2Phase -= 1.0f;
        float rawOp3 = Math::fastSin(PI_X2 * mod2Phase);

        // ── 5. OP2 oscillator ─────────────────────────────────────────────────
        float fbPhase = modFbSmooth * (float)(M_PI * 0.25);

        modPhase += op2Freq * sampleRateDiv;
        if (modPhase > 1.0f) modPhase -= 1.0f;

        int alg = (int)(algorithm.value + 0.5f);

        // ── 6. CARRIER ────────────────────────────────────────────────────────
        phase += carrierFreq * sampleRateDiv;
        if (phase > 1.0f) phase -= 1.0f;

        float sig = 0.0f;
        float rawOp2 = 0.0f;

        switch (alg) {
        case 2: {
            // Parallel: Op2 and Op3 independently modulate carrier
            rawOp2 = Math::fastSin(PI_X2 * modPhase + fbPhase);
            sig = Math::fastSin(PI_X2 * phase + modLvl * rawOp2 + mod2Lvl * rawOp3);
            sig *= carLvl;
            break;
        }
        case 3: {
            // Branch: Op3→Op2 (baked) + Op3 also hits carrier
            float op2ModInput = mod2Lvl * rawOp3;
            rawOp2 = Math::fastSin(PI_X2 * modPhase + fbPhase + op2ModInput);
            sig = Math::fastSin(PI_X2 * phase + modLvl * rawOp2 + mod2Lvl * rawOp3);
            sig *= carLvl;
            break;
        }
        case 4: {
            // Additive: Op2→Carrier, Op3 = free sine carrier
            // mod2Lvl already computed above; divide out INDEX_SCALE for amplitude use
            rawOp2 = Math::fastSin(PI_X2 * modPhase + fbPhase);
            sig = Math::fastSin(PI_X2 * phase + modLvl * rawOp2) * carLvl
                + Math::fastSin(PI_X2 * mod2Phase) * (mod2Lvl / INDEX_SCALE);
            sig *= 0.5f;
            break;
        }
        default: { // 1 — Serial
            float op2ModInput = mod2Lvl * rawOp3;
            rawOp2 = Math::fastSin(PI_X2 * modPhase + fbPhase + op2ModInput);
            sig = Math::fastSin(PI_X2 * phase + modLvl * rawOp2);
            sig *= carLvl;
            break;
        }
        }

        modFbSmooth += 0.3f * (rawOp2 - modFbSmooth);

        // ── 7. VELOCITY ───────────────────────────────────────────────────────
        sig *= velocity;

        // ── 8. FILTER ─────────────────────────────────────────────────────────
        sig = CLAMP(sig, -1.0f, 1.0f);
        float dynCutoff = CLAMP(cutoff.value + lfoOut * lfoToCutoff.value, -100.0f, 100.0f);
        sig = applyMorphFilter(sig, dynCutoff, resonance.value * 0.01f);

        // ── 9. BUFFERED FX ────────────────────────────────────────────────────
        return bufferedFxProcess(sig);
    }
};