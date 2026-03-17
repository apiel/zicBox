#pragma once

#include "audio/effects/applyDrive.h"
#include "audio/effects/applyWaveshape.h"
#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"
#include "helpers/clamp.h"

#include <algorithm>
#include <cmath>
#include <cstring>

// 2-operator FM synth (carrier + modulator) with feedback on the modulator.
// Operator envelope model: ADSR per operator.
// Filter: SVF morphing LP→bypass→HP via a single Cutoff param [-100 % .. +100 %].
// Vel → Mod Index: velocity scales FM depth (DX7-style expressiveness).

class SynthFm23 : public EngineBase<SynthFm23> {

public:
    static constexpr int DELAY_BUF_SIZE = 48000; // 1 s @ 48 kHz
    static constexpr int REVERB_BUF_SIZE = 16384;

protected:
    const float sampleRate;
    const float sampleRateDiv;

    FilterSVF svfFilter;

    // ── Oscillator state ────────────────────────────────────────────────────
    float phase = 0.0f; // carrier phase [0,1)
    float modPhase = 0.0f; // modulator phase [0,1)
    float lfoPhase = 0.0f;

    float currentFreq = 440.0f;
    float targetFreq = 440.0f;

    // ── Voice state ─────────────────────────────────────────────────────────
    float velocity = 1.0f;
    bool gateOpen = false;

    // ── Operator envelopes ──────────────────────────────────────────────────
    // State: 0=off  1=attack  2=decay  3=sustain  4=release
    float carEnv = 0.0f;
    int carStage = 0;
    float modEnv = 0.0f;
    int modStage = 0;

    // ── Modulator feedback ──────────────────────────────────────────────────
    float modFbPrev = 0.0f;

    // ── Delay ───────────────────────────────────────────────────────────────
    float* delayBuf = nullptr;
    int delayWrite = 0;
    float dlyFbSmooth = 0.0f;

    // ── Reverb ──────────────────────────────────────────────────────────────
    float* reverbBuf = nullptr;

    static constexpr int COMB_LEN[4] = { 1559, 1617, 1685, 1751 };
    static constexpr int AP_LEN[3] = { 347, 113, 37 };

    int combOff[4] = {};
    int apOff[3] = {};
    int combIdx[8] = {};
    int apIdx[4] = {};
    float combFb[8] = {};

    // ── Helpers ─────────────────────────────────────────────────────────────
    static float lerp(float a, float b, float t) { return a + t * (b - a); }

    float tau(float ms) const
    {
        return (ms < 0.01f) ? 0.0f : Math::exp(-1.0f / (sampleRate * ms * 0.001f));
    }

    float attackRate(float ms) const
    {
        if (ms < 0.01f) return 1.0f;
        return 1.0f / (sampleRate * ms * 0.001f);
    }

    float adsrTick(float& env, int& stage,
        float attackMs, float decayMs,
        float sustainLvl, float releaseMs,
        bool gate)
    {
        switch (stage) {
        case 0:
            env = 0.0f;
            break;
        case 1: // attack
            env += attackRate(attackMs);
            if (env >= 1.0f) {
                env = 1.0f;
                stage = 2;
            }
            break;
        case 2: // decay
            env *= tau(decayMs);
            if (env <= sustainLvl + 0.001f) {
                env = sustainLvl;
                stage = 3;
            }
            break;
        case 3: // sustain
            env = sustainLvl;
            if (!gate) stage = 4;
            break;
        case 4: // release
            env *= tau(releaseMs);
            if (env < 0.0001f) {
                env = 0.0f;
                stage = 0;
            }
            break;
        }
        return env;
    }

    // ── LP / bypass / HP morphing filter ─────────────────────────────────────
    // cutoffParam: -100 % → full LP … 0 → signal passes dry … +100 % → full HP
    //
    // Strategy:
    //   • The SVF always runs at a fixed normalised frequency derived from
    //     |cutoffParam|, so the resonance peak frequency tracks the knob position.
    //   • LP and HP outputs are blended with a crossfade weight t ∈ [0,1]:
    //       t = (cutoffParam + 100) / 200   → 0 = LP, 0.5 = equal mix, 1 = HP
    //   • The blend is then lerped against the dry signal with depth = |cutoffParam|/100,
    //     so dead-centre (0) gives a perfectly transparent bypass without any
    //     discontinuity or click.
    float applyMorphFilter(float sig, float cutoffParam, float res)
    {
        float absC = std::abs(cutoffParam); // 0..100

        if (absC < 0.5f) return sig; // bypass – negligible effect

        // |cutoffParam| 0..100 → normalised cutoff 0.01..0.99
        float normCutoff = 0.01f + absC * 0.0098f;
        float normRes = CLAMP(res, 0.0f, 0.98f);

        svfFilter.setCutoff(normCutoff);
        svfFilter.setResonance(normRes);
        auto out = svfFilter.process12(sig); // {lp, bp, hp}

        // Crossfade LP→HP: t=0 at cutoffParam=-100, t=1 at cutoffParam=+100
        float t = CLAMP((cutoffParam + 100.0f) * 0.005f, 0.0f, 1.0f);
        float filtered = lerp(out.lp, out.hp, t);

        // Depth fades in from bypass (0) to fully filtered (1) with |cutoffParam|
        float depth = absC * 0.01f;
        return lerp(sig, filtered, depth);
    }

    // ── FX helpers ──────────────────────────────────────────────────────────
    float reverbProcess(float in, float mix, float size, float damp)
    {
        if (mix < 0.001f) return in;

        float decay = 0.7f + size * 0.28f;
        float d = 0.2f + damp * 0.7f;
        float invD = 1.0f - d;
        float wet = 0.0f;

        for (int c = 0; c < 4; ++c) {
            float* bufStart = &reverbBuf[combOff[c]];
            int idx = combIdx[c];

            float delayed = bufStart[idx];
            combFb[c] = delayed * invD + combFb[c] * d;
            bufStart[idx] = in + combFb[c] * decay;
            if (++idx >= COMB_LEN[c]) idx = 0;
            combIdx[c] = idx;
            wet += delayed;
        }
        wet *= 0.25f;

        for (int a = 0; a < 3; ++a) {
            float* bufStart = &reverbBuf[apOff[a]];
            int idx = apIdx[a];

            float delayed = bufStart[idx];
            float v = wet + delayed * 0.5f;
            bufStart[idx] = v;
            wet = delayed - v * 0.5f;
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

        sig = lerp(sig, sig + delayed * 0.7f, dlyMix.value * 0.01f);
        return sig;
    }

    float bufferedFxProcess(float sig)
    {
        sig = delayProcess(sig);
        sig = reverbProcess(sig, reverbMix.value * 0.01f, reverbSize.value * 0.01f, reverbDamp.value * 0.01f);
        return sig;
    }

public:
    // ── Parameter table (24 params) ──────────────────────────────────────────
    //   [0]  Frequency      – base frequency in Hz
    //   [1]  Ratio          – modulator : carrier ratio
    //   [2]  Mod Index      – FM depth (0..20)
    //   [3]  Feedback       – modulator self-feedback (%)
    //   [4]  Vel Index      – velocity → mod index scaling (%)
    //   [5]  Car Attack     – carrier envelope attack  (ms)
    //   [6]  Car Decay      – carrier envelope decay   (ms)
    //   [7]  Car Sust       – carrier envelope sustain (%)
    //   [8]  Car Rel        – carrier envelope release (ms)
    //   [9]  Mod Attack     – modulator envelope attack  (ms)
    //   [10] Mod Decay      – modulator envelope decay   (ms)
    //   [11] Mod Sust       – modulator envelope sustain (%)
    //   [12] Mod Rel        – modulator envelope release (ms)
    //   [13] LFO Rate       – LFO frequency (Hz)
    //   [14] LFO Pitch      – LFO → carrier pitch depth (%)
    //   [15] LFO Index      – LFO → mod index depth (%)
    //   [16] Cutoff         – filter: -100 % LP … 0 bypass … +100 % HP
    //   [17] Resonance      – filter resonance (%)
    //   [18] Reverb Mix     – reverb wet/dry (%)
    //   [19] Rvb Size       – reverb room size (%)
    //   [20] Rvb Damp       – reverb damping (%)
    //   [21] Dly Mix        – delay wet/dry (%)
    //   [22] Dly Time       – delay time (ms)
    //   [23] Dly Fdbk       – delay feedback (%)

    Param params[24] = {
        { .label = "Frequency", .unit = "Hz", .value = 440.0f, .min = 20.0f, .max = 2000.0f, .step = 0.5f }, // 0
        { .label = "Ratio", .unit = "x", .value = 2.0f, .min = 0.25f, .max = 16.0f, .step = 0.01f }, // 1
        { .label = "Mod Index", .unit = "", .value = 3.0f, .min = 0.0f, .max = 10.0f, .step = 0.005f }, // 2
        { .label = "Feedback", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f }, // 3
        { .label = "Vel Index", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f }, // 4
        { .label = "Car Attack", .unit = "ms", .value = 5.0f, .min = 0.5f, .max = 2000.0f, .step = 1.0f }, // 5
        { .label = "Car Decay", .unit = "ms", .value = 300.0f, .min = 5.0f, .max = 4000.0f, .step = 5.0f }, // 6
        { .label = "Car Sust", .unit = "%", .value = 70.0f, .min = 0.0f, .max = 100.0f }, // 7
        { .label = "Car Rel", .unit = "ms", .value = 200.0f, .min = 5.0f, .max = 4000.0f, .step = 5.0f }, // 8
        { .label = "Mod Attack", .unit = "ms", .value = 2.0f, .min = 0.5f, .max = 2000.0f, .step = 1.0f }, // 9
        { .label = "Mod Decay", .unit = "ms", .value = 150.0f, .min = 5.0f, .max = 4000.0f, .step = 5.0f }, // 10
        { .label = "Mod Sust", .unit = "%", .value = 20.0f, .min = 0.0f, .max = 100.0f }, // 11
        { .label = "Mod Rel", .unit = "ms", .value = 100.0f, .min = 5.0f, .max = 4000.0f, .step = 5.0f }, // 12
        { .label = "LFO Rate", .unit = "Hz", .value = 2.0f, .min = 0.05f, .max = 30.0f, .step = 0.05f }, // 13
        { .label = "LFO Pitch", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f }, // 14
        { .label = "LFO Index", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f }, // 15
        { .label = "Cutoff", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f }, // 16
        { .label = "Resonance", .unit = "%", .value = 20.0f, .min = 0.0f, .max = 100.0f }, // 17
        { .label = "Reverb Mix", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f }, // 18
        { .label = "Rvb Size", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f }, // 19
        { .label = "Rvb Damp", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f }, // 20
        { .label = "Dly Mix", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f }, // 21
        { .label = "Dly Time", .unit = "ms", .value = 125.0f, .min = 10.0f, .max = 1000.0f, .step = 5.0f }, // 22
        { .label = "Dly Fdbk", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f }, // 23
    };

    // Named references
    Param& freq = params[0];
    Param& ratio = params[1];
    Param& modIndex = params[2];
    Param& feedback = params[3];
    Param& velIndex = params[4]; // Vel → Mod Index
    Param& carAttack = params[5];
    Param& carDecay = params[6];
    Param& carSustain = params[7];
    Param& carRelease = params[8];
    Param& modAttack = params[9];
    Param& modDecay = params[10];
    Param& modSustain = params[11];
    Param& modRelease = params[12];
    Param& lfoRate = params[13];
    Param& lfoToPitch = params[14];
    Param& lfoToIndex = params[15];
    Param& cutoff = params[16]; // -100 % LP … 0 bypass … +100 % HP
    Param& resonance = params[17];
    Param& reverbMix = params[18];
    Param& reverbSize = params[19];
    Param& reverbDamp = params[20];
    Param& dlyMix = params[21];
    Param& dlyTime = params[22];
    Param& dlyFdbk = params[23];

    SynthFm23(float sr, float* dlBuf, float* rvBuf)
        : EngineBase(Synth, "Fm23", params)
        , sampleRate(sr)
        , sampleRateDiv(1.0f / sr)
        , delayBuf(dlBuf)
        , reverbBuf(rvBuf)
    {
        if (delayBuf) {
            for (int i = 0; i < DELAY_BUF_SIZE; ++i)
                delayBuf[i] = 0.0f;
        }

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
        currentFreq = targetFreq; // no glide in FM engine

        gateOpen = true;
        carStage = 1;
        modStage = 1;
    }

    void noteOffImpl(uint8_t)
    {
        gateOpen = false;
    }

    float sampleImpl()
    {
        if (carStage == 0 && !gateOpen)
            return bufferedFxProcess(0.0f);

        // ── 1. LFO ──────────────────────────────────────────────────────────
        lfoPhase += lfoRate.value * sampleRateDiv;
        if (lfoPhase > 1.0f) lfoPhase -= 1.0f;
        float lfoOut = Math::fastSin(PI_X2 * lfoPhase);

        // ── 2. ENVELOPES ────────────────────────────────────────────────────
        float carLvl = adsrTick(carEnv, carStage,
            carAttack.value, carDecay.value,
            carSustain.value * 0.01f, carRelease.value,
            gateOpen);

        float modLvl = adsrTick(modEnv, modStage,
            modAttack.value, modDecay.value,
            modSustain.value * 0.01f, modRelease.value,
            gateOpen);

        // ── 3. MODULATOR ────────────────────────────────────────────────────
        float carrierFreq = currentFreq * (1.0f + lfoOut * lfoToPitch.value * 0.005f);
        float modulatorFreq = carrierFreq * ratio.value;

        float fb = feedback.value * 0.01f * 0.5f; // scale to avoid blowup
        float fbAmount = modFbPrev * fb;

        modPhase += modulatorFreq * sampleRateDiv;
        if (modPhase > 1.0f) modPhase -= 1.0f;

        float rawMod = Math::fastSin(PI_X2 * modPhase + fbAmount);
        modFbPrev = rawMod;

        // Vel → Index: lerp between full index (velIndex=0) and vel-scaled index (velIndex=100).
        // At velIndex=100 a soft note (vel≈0) produces near-zero FM depth → bright only when played hard.
        float velScale = lerp(1.0f, velocity, velIndex.value * 0.01f);
        float effectiveIndex = modIndex.value
            * modLvl
            * velScale
            * (1.0f + lfoOut * lfoToIndex.value * 0.01f);

        // ── 4. CARRIER ──────────────────────────────────────────────────────
        phase += carrierFreq * sampleRateDiv;
        if (phase > 1.0f) phase -= 1.0f;

        float sig = Math::fastSin(PI_X2 * phase + PI_X2 * effectiveIndex * rawMod);

        // ── 5. AMP ENVELOPE ─────────────────────────────────────────────────
        sig *= carLvl * velocity;

        // ── 6. FILTER (LP / bypass / HP morph) ──────────────────────────────
        sig = CLAMP(sig, -1.0f, 1.0f);
        sig = applyMorphFilter(sig, cutoff.value, resonance.value * 0.01f);

        // ── 7. BUFFERED FX ───────────────────────────────────────────────────
        sig = bufferedFxProcess(sig);

        return sig;
    }
};