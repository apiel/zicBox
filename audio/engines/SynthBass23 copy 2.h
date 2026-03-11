#pragma once

#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"

#include <algorithm>
#include <cmath>
#include <cstring>

// ============================================================================
//  SynthBass23  —  Acid / Mental Tek bass engine
//  Inspired by Transistor Bass (FL Studio) / Roland TB-303
//
//  Self-contained: no external reverb/drive helpers needed.
//
//  Constructor: SynthBass23(float sampleRate, float* delayBuf, float* reverbBuf)
//    delayBuf  — caller allocates DELAY_BUF_SIZE  floats
//    reverbBuf — caller allocates REVERB_BUF_SIZE floats
// ============================================================================

class SynthBass23 : public EngineBase<SynthBass23> {

public:
    // ── Buffer sizes for the caller to allocate ──────────────────────────────
    static constexpr int DELAY_BUF_SIZE = 48000; // 1 s @ 48 kHz
    static constexpr int REVERB_BUF_SIZE = 16384; // covers all comb+allpass

protected:
    const float sampleRate;
    const float sampleRateDiv;

    // ── Voice ────────────────────────────────────────────────────────────────
    float velocity = 1.0f;
    float phase = 0.0f;
    float currentFreq = 110.0f;
    float targetFreq = 110.0f;
    bool gateOpen = false;
    bool accented = false;

    // ── Envelopes ────────────────────────────────────────────────────────────
    float vcfEnv = 0.0f;
    float ampEnv = 0.0f;
    float accentVcf = 0.0f; // decaying accent boost for cutoff
    float accentVca = 0.0f; // decaying accent boost for volume
    float vcaSmoothSt = 0.0f; // VCA click-smoother one-pole state

    // ── Filter  (3-pass 4-pole Moog ladder) ─────────────────────────────────
    float flt[3][4] = {};
    float fltFb[3] = {};
    float hpState = 0.0f;

    // ── LFO ─────────────────────────────────────────────────────────────────
    float lfoPhase = 0.0f;

    // ── Delay ───────────────────────────────────────────────────────────────
    float* delayBuf = nullptr;
    int delayWrite = 0;
    float dlyFbSmooth = 0.0f;

    // ── Self-contained Schroeder reverb ─────────────────────────────────────
    // 8 parallel combs + 4 series allpass, all packed into one flat buffer.
    float* reverbBuf = nullptr;

    static constexpr int COMB_LEN[8] = { 1559, 1617, 1685, 1751, 1805, 1871, 1945, 2017 };
    static constexpr int AP_LEN[4] = { 347, 113, 37, 11 };

    int combOff[8] = {};
    int apOff[4] = {};
    int combIdx[8] = {};
    int apIdx[4] = {};
    float combFb[8] = {}; // per-comb LP state (damping)

    // ── Helpers ──────────────────────────────────────────────────────────────
    static float lerp(float a, float b, float t) { return a + t * (b - a); }

    float tau(float ms) const
    {
        return (ms < 0.01f) ? 0.0f : Math::exp(-1.0f / (sampleRate * ms * 0.001f));
    }

    static float noteToFreq(uint8_t note)
    {
        return 440.0f * std::pow(2.0f, (static_cast<float>(note) - 69.0f) / 12.0f);
    }

    // ── 4-pole Moog ladder — one pass ────────────────────────────────────────
    // Nonlinear tanh at input + feedback for extreme resonance screaming.
    float ladderPass(float in, float cutHz, float res, int pass)
    {
        float f = std::min(cutHz * sampleRateDiv * 1.16f, 0.48f);
        float k = f * (2.0f - f);
        float r = res * 4.1f; // self-oscillation kicks in near res=1.0

        float x = in - r * std::tanh(fltFb[pass] * 0.7f);
        x = std::tanh(x);

        float* s = flt[pass];
        for (int i = 0; i < 4; ++i) {
            s[i] = s[i] + k * (std::tanh(x) - s[i]);
            x = s[i];
        }
        fltFb[pass] = s[3];
        return s[3];
    }

    // ── Custom distortion: pre-gain + wavefolder + asymmetric clip ───────────
    // distAmt 0..1, color 0..1 (fold vs. clip blend), bias -1..1 (asymmetry)
    float distort(float in, float distAmt, float color, float bias) const
    {
        if (distAmt < 0.001f) return in;

        float gain = 1.0f + distAmt * 14.0f;
        float x = in * gain + bias * 0.3f;

        // Cosine wavefolder: produces even harmonics, bell-like metallic texture
        float folded = Math::fastSin(x * (1.0f + color * 2.5f) * 0.5f * (float)M_PI);
        x = lerp(x, folded, color);

        // Asymmetric soft-clip: tanh+ on positive, harder rational clip on negative
        float clipped = (x >= 0.0f) ? std::tanh(x) : x / (1.0f - x * 0.4f);

        // Compensate loudness increase from gain
        return clipped / (1.0f + distAmt * 3.0f);
    }

    // ── Schroeder reverb ─────────────────────────────────────────────────────
    float reverbProcess(float in, float mix, float size, float damp)
    {
        if (!reverbBuf || mix < 0.001f) return in;

        float decay = 0.7f + size * 0.28f;
        float d = 0.2f + damp * 0.7f;

        float wet = 0.0f;

        // 8 parallel comb filters with LP damping inside the loop
        for (int c = 0; c < 8; ++c) {
            int len = COMB_LEN[c];
            int idx = combIdx[c];
            float delayed = reverbBuf[combOff[c] + idx];
            combFb[c] = delayed * (1.0f - d) + combFb[c] * d;
            reverbBuf[combOff[c] + idx] = in + combFb[c] * decay;
            combIdx[c] = (idx + 1) % len;
            wet += delayed;
        }
        wet *= (1.0f / 8.0f);

        // 4 series allpass
        for (int a = 0; a < 4; ++a) {
            int len = AP_LEN[a];
            int idx = apIdx[a];
            float delayed = reverbBuf[apOff[a] + idx];
            float v = wet + delayed * 0.5f;
            reverbBuf[apOff[a] + idx] = v;
            wet = delayed - wet * 0.5f;
            apIdx[a] = (idx + 1) % len;
        }

        return in + wet * mix;
    }

public:
    // ════════════════════════════════════════════════════════════════════════
    //  PARAMS  (24 — 2 pages of 12)
    //  PAGE 1: Classic 303 core
    //  PAGE 2: Distortion, Reverb, Delay, LFO + Glide/VCA tweaks
    // ════════════════════════════════════════════════════════════════════════
    Param params[24] = {
        // ── PAGE 1 ───────────────────────────────────────────────────────────
        // 0  Tuning  (coarse, ±1 octave)
        { .label = "Tuning", .unit = "semi", .value = 0.0f, .min = -12.0f, .max = 12.0f, .step = 1.0f },
        // 1  Waveform  0=Square → 100=Sawtooth  (Transistor Bass convention)
        { .label = "Waveform", .unit = "Sq-Saw", .value = 0.0f },
        // 2  Pulse Width (only heard on square side; LFO modulates this)
        { .label = "PW", .unit = "%", .value = 50.0f, .min = 5.0f, .max = 95.0f },
        // 3  LFO → PW  (classic 303 hardwired LFO destination)
        { .label = "LFO\x88PW", .unit = "%", .value = 0.0f },
        // 4  Filter cutoff
        { .label = "Cutoff", .unit = "Hz", .value = 600.0f, .min = 30.0f, .max = 12000.0f, .step = 10.0f },
        // 5  Resonance  (self-oscillates above ~90%)
        { .label = "Resonance", .unit = "%", .value = 50.0f },
        // 6  Filter envelope amount (bipolar)
        { .label = "Env Mod", .unit = "%", .value = 50.0f, .min = -100.0f },
        // 7  Filter envelope / amp decay
        { .label = "Decay", .unit = "ms", .value = 200.0f, .min = 10.0f, .max = 2000.0f, .step = 5.0f },
        // 8  Min Envelope Decay — THE character param (see Transistor Bass docs)
        { .label = "Min Decay", .unit = "ms", .value = 30.0f, .min = 1.0f, .max = 200.0f, .step = 1.0f },
        // 9  Accent amount  (velocity > 0.75 → accented note)
        { .label = "Accent", .unit = "%", .value = 60.0f },
        // 10 HP cutoff  (original 303 has fixed HP; here it's tunable)
        { .label = "HP", .unit = "%", .value = 20.0f },
        // 11 Filter Key Follow  (negative←0→positive, centred = 0)
        { .label = "KeyFollow", .unit = "%", .value = 0.0f, .min = -100.0f },

        // ── PAGE 2 ───────────────────────────────────────────────────────────
        // 12 Glide (portamento)
        { .label = "Glide", .unit = "ms", .value = 0.0f, .max = 1000.0f, .step = 5.0f },
        // 13 VCA Smooth  (0=clicky 303 attack, 100=smooth)
        { .label = "VCA Smooth", .unit = "%", .value = 0.0f },
        // 14 LFO Rate
        { .label = "LFO Rate", .unit = "Hz", .value = 2.0f, .min = 0.05f, .max = 20.0f, .step = 0.05f },

        // 15 Distortion amount
        { .label = "Dist Amt", .unit = "%", .value = 0.0f },
        // 16 Dist Color  (0=soft-clip only, 100=full wavefold)
        { .label = "Dist Color", .unit = "%", .value = 30.0f },
        // 17 Dist Bias  (asymmetry / grit; centred = 0)
        { .label = "Dist Bias", .unit = "%", .value = 0.0f, .min = -100.0f },

        // 18 Reverb mix
        { .label = "Reverb Mix", .unit = "%", .value = 0.0f },
        // 19 Reverb size / tail length
        { .label = "Rvb Size", .unit = "%", .value = 50.0f },
        // 20 Reverb damping (high = dark/warm)
        { .label = "Rvb Damp", .unit = "%", .value = 50.0f },

        // 21 Delay time
        { .label = "Dly Time", .unit = "ms", .value = 125.0f, .min = 10.0f, .max = 1000.0f, .step = 5.0f },
        // 22 Delay feedback
        { .label = "Dly Fdbk", .unit = "%", .value = 0.0f },
        // 23 Delay mix
        { .label = "Dly Mix", .unit = "%", .value = 0.0f },
    };

    // ── Named references ─────────────────────────────────────────────────────
    Param& tuning = params[0];
    Param& waveform = params[1];
    Param& pw = params[2];
    Param& lfoToPW = params[3];
    Param& cutoff = params[4];
    Param& resonance = params[5];
    Param& envMod = params[6];
    Param& decayTime = params[7];
    Param& minDecay = params[8];
    Param& accentAmt = params[9];
    Param& hpCutoff = params[10];
    Param& keyFollow = params[11];
    Param& glide = params[12];
    Param& vcaSmooth = params[13];
    Param& lfoRate = params[14];
    Param& distAmt = params[15];
    Param& distColor = params[16];
    Param& distBias = params[17];
    Param& reverbMix = params[18];
    Param& reverbSize = params[19];
    Param& reverbDamp = params[20];
    Param& dlyTime = params[21];
    Param& dlyFdbk = params[22];
    Param& dlyMix = params[23];

    // ── Constructor ──────────────────────────────────────────────────────────
    SynthBass23(float sr, float* dlBuf, float* rvBuf)
        : EngineBase(Synth, "Bass23", params)
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
            for (int c = 0; c < 8; ++c) {
                combOff[c] = pos;
                pos += COMB_LEN[c];
            }
            for (int a = 0; a < 4; ++a) {
                apOff[a] = pos;
                pos += AP_LEN[a];
            }
            for (int i = 0; i < REVERB_BUF_SIZE; ++i)
                reverbBuf[i] = 0.0f;
        }

        init();
    }

    // ── noteOn ───────────────────────────────────────────────────────────────
    void noteOnImpl(uint8_t note, float vel)
    {
        velocity = vel;
        accented = (vel > 0.75f);
        targetFreq = noteToFreq(note) * std::pow(2.0f, tuning.value / 12.0f);

        // Keep current frequency for glide only if gate was already open
        if (!gateOpen || glide.value < 0.5f)
            currentFreq = targetFreq;

        gateOpen = true;

        // Retrigger filter env to top
        vcfEnv = 1.0f;
        // Amp holds at 1 while gate open
        ampEnv = 1.0f;

        if (accented) {
            float a = accentAmt.value * 0.01f;
            accentVcf = a * 0.8f;
            accentVca = a * 0.35f;
        }
    }

    // ── noteOff ──────────────────────────────────────────────────────────────
    void noteOffImpl(uint8_t)
    {
        gateOpen = false;
        // ampEnv will decay from here; vcfEnv continues its own decay
    }

    // ── sample ───────────────────────────────────────────────────────────────
    float sampleImpl()
    {
        if (ampEnv < 0.0001f && !gateOpen) return 0.0f;

        // ── 1. GLIDE ────────────────────────────────────────────────────────
        if (glide.value > 0.5f) {
            float c = tau(glide.value);
            currentFreq += (1.0f - c) * (targetFreq - currentFreq);
        } else {
            currentFreq = targetFreq;
        }

        // ── 2. LFO (sine) ────────────────────────────────────────────────────
        lfoPhase += lfoRate.value * sampleRateDiv;
        if (lfoPhase > 1.0f) lfoPhase -= 1.0f;
        float lfoOut = Math::fastSin(PI_X2 * lfoPhase);

        // ── 3. OSCILLATOR ───────────────────────────────────────────────────
        phase += currentFreq * sampleRateDiv;
        if (phase > 1.0f) phase -= 1.0f;

        float sawWave = 2.0f * phase - 1.0f;

        float pwMod = pw.value * 0.01f + lfoOut * (lfoToPW.value * 0.01f) * 0.45f;
        pwMod = std::max(0.02f, std::min(pwMod, 0.98f));
        float sqWave = (phase < pwMod) ? 1.0f : -1.0f;

        // 0=Square → 100=Saw
        float osc = lerp(sqWave, sawWave, waveform.value * 0.01f);

        // ── 4. PRE-FILTER DISTORTION ────────────────────────────────────────
        // Pre-filter placement = harmonics get sculpted by the ladder.
        osc = distort(osc,
            distAmt.value * 0.01f,
            distColor.value * 0.01f,
            distBias.value * 0.01f);

        // ── 5. FILTER ENVELOPE (AD) ─────────────────────────────────────────
        // Always decays; noteOn snaps it to 1.
        // Effective decay is clamped to minDecay — this shapes accent character.
        float effDecay = std::max(decayTime.value, minDecay.value);
        vcfEnv *= tau(effDecay);

        // Accent envelopes (both decay with fixed ~60 ms time)
        float accentC = tau(60.0f);
        accentVcf *= accentC;
        accentVca *= accentC;

        // ── 6. DYNAMIC CUTOFF ────────────────────────────────────────────────
        float envModHz = envMod.value * 0.01f * 7000.0f;
        float accentHz = accentVcf * 4000.0f;
        float kfHz = (currentFreq - 110.0f) * (keyFollow.value * 0.02f);
        float dynCutoff = cutoff.value + envModHz * vcfEnv + accentHz + kfHz;
        dynCutoff = std::max(30.0f, std::min(dynCutoff, sampleRate * 0.46f));

        // ── 7. RESONANCE ─────────────────────────────────────────────────────
        float res = std::min(resonance.value * 0.01f + accentVcf * 0.25f, 0.99f);

        // ── 8. 3-PASS LADDER ─────────────────────────────────────────────────
        // Three consecutive passes through the 4-pole ladder.
        // Each pass has independent state → combined response is extremely
        // aggressive with screaming, self-oscillating resonance.
        float sig = ladderPass(osc, dynCutoff, res, 0);
        sig = ladderPass(sig * 0.6f, dynCutoff, res, 1);
        sig = ladderPass(sig * 0.6f, dynCutoff, res, 2);
        sig *= 2.5f; // compensate attenuation

        // ── 9. HP FILTER ─────────────────────────────────────────────────────
        float hpCoeff = 0.0005f + hpCutoff.value * 0.0005f;
        hpState += hpCoeff * (sig - hpState);
        sig = (sig - hpState) * (1.0f + hpCutoff.value * 0.015f);

        // ── 10. AMP ENVELOPE ────────────────────────────────────────────────
        if (gateOpen) {
            ampEnv = 1.0f;
        } else {
            // Amp decay mirrors filter decay (classic 303 behaviour);
            // accented notes release a bit faster
            float ampDecMs = std::max(decayTime.value, minDecay.value)
                * (accented ? 0.6f : 1.2f);
            ampEnv *= tau(ampDecMs);
        }

        // VCA smooth: one-pole to soften note attack/release clicks
        float smoothT = vcaSmooth.value * 0.15f + 0.05f; // 0.05..15 ms
        vcaSmoothSt += (1.0f - tau(smoothT)) * (ampEnv - vcaSmoothSt);
        float smoothedAmp = lerp(ampEnv, vcaSmoothSt, vcaSmooth.value * 0.01f);

        sig *= smoothedAmp * (velocity + accentVca);

        // ── 11. DELAY ───────────────────────────────────────────────────────
        if (delayBuf) {
            int delaySmp = std::max(1, std::min((int)(dlyTime.value * 0.001f * sampleRate), DELAY_BUF_SIZE - 1));
            int readIdx = (delayWrite - delaySmp + DELAY_BUF_SIZE) % DELAY_BUF_SIZE;
            float delayed = delayBuf[readIdx];

            float fbTarget = dlyFdbk.value * 0.01f * 0.85f;
            dlyFbSmooth += 0.001f * (fbTarget - dlyFbSmooth);

            delayBuf[delayWrite] = sig + delayed * dlyFbSmooth;
            delayWrite = (delayWrite + 1) % DELAY_BUF_SIZE;

            sig = lerp(sig, sig + delayed * 0.7f, dlyMix.value * 0.01f);
        }

        // ── 12. REVERB ──────────────────────────────────────────────────────
        sig = reverbProcess(sig,
            reverbMix.value * 0.01f,
            reverbSize.value * 0.01f,
            reverbDamp.value * 0.01f);

        return sig;
    }
};