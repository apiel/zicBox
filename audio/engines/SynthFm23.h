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
// Operator envelope model: Attack / Decay / Sustain / Release (ADSR) per operator.
// Additional features: LFO → pitch/mod-index, HP, drive, waveshape, delay, reverb.

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

    // ── Operator envelopes (carrier = amp, modulator = mod-index scale) ─────
    // Each envelope runs: Attack → Decay → Sustain hold → Release
    // State: 0=off, 1=attack, 2=decay, 3=sustain, 4=release
    float carEnv = 0.0f;
    int carStage = 0;
    float modEnv = 0.0f;
    int modStage = 0;

    // ── Modulator feedback ──────────────────────────────────────────────────
    float modFbPrev = 0.0f; // previous modulator output for self-feedback

    // ── HP filter ───────────────────────────────────────────────────────────
    float hpState = 0.0f;

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

    // Linear ramp coefficient for attack (reaches 1 in `ms` milliseconds)
    float attackRate(float ms) const
    {
        if (ms < 0.01f) return 1.0f;
        return 1.0f / (sampleRate * ms * 0.001f);
    }

    // Process a single ADSR stage and return the current envelope value.
    // `env` and `stage` are in/out.
    float adsrTick(float& env, int& stage,
        float attackMs, float decayMs,
        float sustainLvl, float releaseMs,
        bool gate)
    {
        switch (stage) {
        case 0: // off
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

    // ── FX helpers (identical pattern to SynthBass23) ───────────────────────
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
    // ── Parameter table (max 24) ─────────────────────────────────────────────
    //   [0]  Frequency        – base frequency in Hz
    //   [1]  Ratio            – modulator : carrier frequency ratio
    //   [2]  Mod Index        – FM modulation depth (0..20)
    //   [3]  Feedback         – modulator self-feedback amount (0..100 %)
    //   [4]  Car Attack       – carrier envelope attack (ms)
    //   [5]  Car Decay        – carrier envelope decay  (ms)
    //   [6]  Car Sustain      – carrier envelope sustain level (%)
    //   [7]  Car Release      – carrier envelope release (ms)
    //   [8]  Mod Attack       – modulator envelope attack (ms)
    //   [9]  Mod Decay        – modulator envelope decay  (ms)
    //   [10] Mod Sustain      – modulator envelope sustain level (%)
    //   [11] Mod Release      – modulator envelope release (ms)
    //   [12] Glide            – portamento time (ms)
    //   [13] LFO Rate         – LFO frequency (Hz)
    //   [14] LFO → Pitch      – LFO modulation depth to carrier pitch (%)
    //   [15] LFO → Index      – LFO modulation depth to mod-index (%)
    //   [16] HP               – high-pass cutoff (%)
    //   [17] Drive            – overdrive / tube saturation (%)
    //   [18] Waveshape        – wavefolding amount (%)
    //   [19] Reverb Mix       – wet/dry (%)
    //   [20] Rvb Size         – reverb room size (%)
    //   [21] Rvb Damp         – reverb damping (%)
    //   [22] Dly Mix          – delay wet/dry (%)
    //   [23] Dly Time         – delay time (ms)  [Fdbk omitted – stays internal]

    Param params[24] = {
        { .label = "Frequency", .unit = "Hz", .value = 440.0f, .min = 20.0f, .max = 2000.0f, .step = 0.5f }, // 0
        { .label = "Ratio", .unit = "x", .value = 2.0f, .min = 0.25f, .max = 16.0f, .step = 0.01f }, // 1
        { .label = "Mod Index", .unit = "", .value = 3.0f, .min = 0.0f, .max = 20.0f, .step = 0.05f }, // 2
        { .label = "Feedback", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f }, // 3
        { .label = "Car Attack", .unit = "ms", .value = 5.0f, .min = 0.5f, .max = 2000.0f, .step = 1.0f }, // 4
        { .label = "Car Decay", .unit = "ms", .value = 300.0f, .min = 5.0f, .max = 4000.0f, .step = 5.0f }, // 5
        { .label = "Car Sustain", .unit = "%", .value = 70.0f, .min = 0.0f, .max = 100.0f }, // 6
        { .label = "Car Release", .unit = "ms", .value = 200.0f, .min = 5.0f, .max = 4000.0f, .step = 5.0f }, // 7
        { .label = "Mod Attack", .unit = "ms", .value = 2.0f, .min = 0.5f, .max = 2000.0f, .step = 1.0f }, // 8
        { .label = "Mod Decay", .unit = "ms", .value = 150.0f, .min = 5.0f, .max = 4000.0f, .step = 5.0f }, // 9
        { .label = "Mod Sustain", .unit = "%", .value = 20.0f, .min = 0.0f, .max = 100.0f }, // 10
        { .label = "Mod Release", .unit = "ms", .value = 100.0f, .min = 5.0f, .max = 4000.0f, .step = 5.0f }, // 11
        { .label = "Glide", .unit = "ms", .value = 0.0f, .min = 0.0f, .max = 1000.0f, .step = 5.0f }, // 12
        { .label = "LFO Rate", .unit = "Hz", .value = 2.0f, .min = 0.05f, .max = 30.0f, .step = 0.05f }, // 13
        { .label = "LFO Pitch", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f }, // 14
        { .label = "LFO Index", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f }, // 15
        { .label = "HP", .unit = "%", .value = 10.0f, .min = 0.0f, .max = 100.0f }, // 16
        { .label = "Waveshape", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f }, // 18
        { .label = "Reverb Mix", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f }, // 19
        { .label = "Rvb Size", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f }, // 20
        { .label = "Rvb Damp", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f }, // 21
        { .label = "Dly Mix", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f }, // 22
        { .label = "Dly Time", .unit = "ms", .value = 125.0f, .min = 10.0f, .max = 1000.0f, .step = 5.0f }, // 23
        { .label = "Dly Fdbk", .unit = "%", .value = 0.0f },
    };

    // Named references for readability
    Param& freq = params[0];
    Param& ratio = params[1];
    Param& modIndex = params[2];
    Param& feedback = params[3];
    Param& carAttack = params[4];
    Param& carDecay = params[5];
    Param& carSustain = params[6];
    Param& carRelease = params[7];
    Param& modAttack = params[8];
    Param& modDecay = params[9];
    Param& modSustain = params[10];
    Param& modRelease = params[11];
    Param& glide = params[12];
    Param& lfoRate = params[13];
    Param& lfoToPitch = params[14];
    Param& lfoToIndex = params[15];
    Param& hpCutoff = params[16];
    Param& waveshape = params[17];
    Param& reverbMix = params[18];
    Param& reverbSize = params[19];
    Param& reverbDamp = params[20];
    Param& dlyMix = params[21];
    Param& dlyTime = params[22];
    Param& dlyFdbk = params[23];

    // Delay feedback is kept as a fixed internal value (0.4) to stay within 24 params.
    // Override via dlyFbSmooth directly if needed.
    static constexpr float DLY_FEEDBACK = 0.40f;

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

        // Frequency relative to MIDI note 60 (Middle C)
        float noteOffset = static_cast<float>(note) - 60.0f;
        targetFreq = freq.value * std::pow(2.0f, noteOffset / 12.0f);

        if (!gateOpen || glide.value < 0.5f)
            currentFreq = targetFreq;

        gateOpen = true;

        // Trigger envelopes
        carStage = 1;
        modStage = 1;
    }

    void noteOffImpl(uint8_t)
    {
        gateOpen = false;
        // Let ADSR move to release stage naturally via gate flag in adsrTick
    }

    float sampleImpl()
    {
        bool carActive = (carStage != 0) || gateOpen;
        bool modActive = (modStage != 0) || gateOpen;

        if (!carActive && !modActive)
            return bufferedFxProcess(0.0f);

        // ── 1. GLIDE ────────────────────────────────────────────────────────
        if (glide.value > 0.5f) {
            float c = tau(glide.value);
            currentFreq += (1.0f - c) * (targetFreq - currentFreq);
        } else {
            currentFreq = targetFreq;
        }

        // ── 2. LFO ──────────────────────────────────────────────────────────
        lfoPhase += lfoRate.value * sampleRateDiv;
        if (lfoPhase > 1.0f) lfoPhase -= 1.0f;
        float lfoOut = Math::fastSin(PI_X2 * lfoPhase);

        // ── 3. ENVELOPES ────────────────────────────────────────────────────
        float carLvl = adsrTick(carEnv, carStage,
            carAttack.value,
            carDecay.value,
            carSustain.value * 0.01f,
            carRelease.value,
            gateOpen);

        float modLvl = adsrTick(modEnv, modStage,
            modAttack.value,
            modDecay.value,
            modSustain.value * 0.01f,
            modRelease.value,
            gateOpen);

        // ── 4. MODULATOR ────────────────────────────────────────────────────
        float carrierFreq = currentFreq * (1.0f + lfoOut * lfoToPitch.value * 0.005f);
        float modulatorFreq = carrierFreq * ratio.value;

        // Self-feedback: modulate phase with previous output
        float fb = feedback.value * 0.01f * 0.5f; // scale to avoid blowup
        float fbAmount = modFbPrev * fb;

        modPhase += modulatorFreq * sampleRateDiv;
        if (modPhase > 1.0f) modPhase -= 1.0f;

        float rawMod = Math::fastSin(PI_X2 * modPhase + fbAmount);
        modFbPrev = rawMod;

        // Effective modulation index scaled by envelope and LFO
        float effectiveIndex = modIndex.value
            * modLvl
            * (1.0f + lfoOut * lfoToIndex.value * 0.01f);

        // Phase modulation applied to carrier (PM ≡ FM for sinusoidal modulator)
        float phaseMod = effectiveIndex * rawMod;

        // ── 5. CARRIER ──────────────────────────────────────────────────────
        phase += carrierFreq * sampleRateDiv;
        if (phase > 1.0f) phase -= 1.0f;

        float sig = Math::fastSin(PI_X2 * phase + PI_X2 * phaseMod);

        // ── 6. AMP ENVELOPE ─────────────────────────────────────────────────
        sig *= carLvl * velocity;

        // ── 7. HP FILTER ─────────────────────────────────────────────────────
        sig = CLAMP(sig, -1.0f, 1.0f);
        float hpCoeff = 0.0005f + hpCutoff.value * 0.0005f;
        hpState += hpCoeff * (sig - hpState);
        sig = (sig - hpState) * (1.0f + hpCutoff.value * 0.015f);

        // ── 8. DRIVE + WAVESHAPE ─────────────────────────────────────────────
        sig = applyWaveshape2(sig, waveshape.value * 0.01f);

        // ── 9. BUFFERED FX ───────────────────────────────────────────────────
        sig = bufferedFxProcess(sig);

        return sig;
    }
};