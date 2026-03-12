#pragma once

#include "audio/engines/EngineBase.h"
#include "audio/filterArray.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"

#include <algorithm>
#include <cmath>
#include <cstring>

class SynthBass23 : public EngineBase<SynthBass23> {

public:
    static constexpr int DELAY_BUF_SIZE = 48000; // 1 s @ 48 kHz
    static constexpr int REVERB_BUF_SIZE = 16384; // covers all comb+allpass

protected:
    const float sampleRate;
    const float sampleRateDiv;

    EffectFilterArray<2> filter;

    float velocity = 1.0f;
    float phase = 0.0f;
    float subPhase = 0.0f; // sub oscillator (one octave below)
    float currentFreq = 110.0f;
    float targetFreq = 110.0f;
    bool gateOpen = false;
    bool accented = false;

    float vcfEnv = 0.0f;
    float ampEnv = 0.0f;
    float accentVcf = 0.0f; // decaying accent boost for cutoff
    float accentVca = 0.0f; // decaying accent boost for volume
    float vcaSmoothSt = 0.0f; // VCA click-smoother one-pole state

    // float flt[3][4] = {};
    // float fltFb[3] = {};
    float hpState = 0.0f;

    float lfoPhase = 0.0f;

    float* delayBuf = nullptr;
    int delayWrite = 0;
    float dlyFbSmooth = 0.0f;
    float* reverbBuf = nullptr;

    static constexpr int COMB_LEN[8] = { 1559, 1617, 1685, 1751, 1805, 1871, 1945, 2017 };
    static constexpr int AP_LEN[4] = { 347, 113, 37, 11 };

    int combOff[8] = {};
    int apOff[4] = {};
    int combIdx[8] = {};
    int apIdx[4] = {};
    float combFb[8] = {}; // per-comb LP state (damping)

    static float lerp(float a, float b, float t) { return a + t * (b - a); }

    float tau(float ms) const
    {
        return (ms < 0.01f) ? 0.0f : Math::exp(-1.0f / (sampleRate * ms * 0.001f));
    }

    static float noteToFreq(uint8_t note)
    {
        return 440.0f * std::pow(2.0f, (static_cast<float>(note) - 69.0f) / 12.0f);
    }

    float distort(float in, float distAmt, float color, float bias) const
    {
        if (distAmt < 0.001f) return in;

        float gain = 1.0f + distAmt * 14.0f;
        float x = in * gain + bias * 0.3f;

        // Cosine wavefolder: produces even harmonics, bell-like metallic texture
        float folded = Math::fastSin(x * (1.0f + color * 2.5f) * 0.5f * (float)M_PI);
        x = lerp(x, folded, color);

        // Asymmetric soft-clip: tanh on positive, harder rational clip on negative
        float clipped = (x >= 0.0f) ? std::tanh(x) : x / (1.0f - x * 0.4f);

        // Compensate loudness increase from gain
        return clipped / (1.0f + distAmt * 3.0f);
    }

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
    Param params[24] = {
        { .label = "Tuning", .unit = "semi", .value = 0.0f, .min = -24.0f, .max = 24.0f, .step = 1.0f },
        { .label = "Waveform", .unit = "Sq-Saw", .value = 0.0f },
        { .label = "Pulse Width", .unit = "%", .value = 50.0f, .min = 5.0f, .max = 95.0f },
        { .label = "Sub Mix", .unit = "%", .value = 50.0f },
        { .label = "Cutoff", .unit = "%", .value = 50.0f },
        { .label = "Resonance", .unit = "%", .value = 20.0f },
        { .label = "Env Mod", .unit = "%", .value = 50.0f, .min = -100.0f },
        { .label = "Decay", .unit = "ms", .value = 200.0f, .min = 10.0f, .max = 2000.0f, .step = 5.0f },
        { .label = "Min Decay", .unit = "ms", .value = 30.0f, .min = 1.0f, .max = 200.0f, .step = 1.0f },
        { .label = "Accent", .unit = "%", .value = 60.0f },
        { .label = "HP", .unit = "%", .value = 20.0f },
        { .label = "TODO KeyFollow", .unit = "%", .value = 0.0f, .min = -100.0f },

        { .label = "LFO PW", .unit = "%", .value = 0.0f },
        { .label = "LFO PW Rate", .unit = "Hz", .value = 2.0f, .min = 0.05f, .max = 20.0f, .step = 0.05f },
        { .label = "Glide", .unit = "ms", .value = 0.0f, .max = 1000.0f, .step = 5.0f },
        { .label = "VCA Smooth", .unit = "%", .value = 0.0f },
        { .label = "Dist Amt", .unit = "%", .value = 0.0f },
        { .label = "Dist Color", .unit = "%", .value = 30.0f },
        { .label = "Reverb Mix", .unit = "%", .value = 0.0f },
        { .label = "Rvb Size", .unit = "%", .value = 50.0f },
        { .label = "Rvb Damp", .unit = "%", .value = 50.0f },
        { .label = "Dly Time", .unit = "ms", .value = 125.0f, .min = 10.0f, .max = 1000.0f, .step = 5.0f },
        { .label = "Dly Fdbk", .unit = "%", .value = 0.0f },
        { .label = "Dly Mix", .unit = "%", .value = 0.0f },
    };

    Param& tuning = params[0];
    Param& waveform = params[1];
    Param& pw = params[2];
    Param& subMix = params[3];
    Param& cutoff = params[4];
    Param& resonance = params[5];
    Param& envMod = params[6];
    Param& decayTime = params[7];
    Param& minDecay = params[8];
    Param& accentAmt = params[9];
    Param& hpCutoff = params[10];
    Param& keyFollow = params[11];
    Param& lfoToPW = params[12];
    Param& lfoRate = params[13];
    Param& glide = params[14];
    Param& vcaSmooth = params[15];
    Param& distAmt = params[16];
    Param& distColor = params[17];
    Param& reverbMix = params[18];
    Param& reverbSize = params[19];
    Param& reverbDamp = params[20];
    Param& dlyTime = params[21];
    Param& dlyFdbk = params[22];
    Param& dlyMix = params[23];

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

    void noteOnImpl(uint8_t note, float vel)
    {
        velocity = vel;
        accented = (vel > 0.75f);
        // Let's remove 1 octave because it's bass :p
        targetFreq = noteToFreq(note - 12) * std::pow(2.0f, tuning.value / 12.0f);

        // Keep current frequency for glide only if gate was already open
        if (!gateOpen || glide.value < 0.5f)
            currentFreq = targetFreq;

        gateOpen = true;
        vcfEnv = 1.0f;
        ampEnv = 1.0f;

        if (accented) {
            float a = accentAmt.value * 0.01f;
            accentVcf = a * 0.8f;
            accentVca = a * 0.35f;
        }
    }

    void noteOffImpl(uint8_t)
    {
        gateOpen = false;
    }

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

        // ── 2. LFO (sine) ───────────────────────────────────────────────────
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

        // Sub oscillator: sine, one octave below, mixed in before filter
        subPhase += (currentFreq * 0.5f) * sampleRateDiv;
        if (subPhase > 1.0f) subPhase -= 1.0f;
        float sub = Math::fastSin(PI_X2 * subPhase);
        osc = lerp(osc, sub, subMix.value * 0.01f);

        // ── 4. PRE-FILTER DISTORTION ────────────────────────────────────────
        osc = distort(osc,
            distAmt.value * 0.01f,
            distColor.value * 0.01f,
            0.0f);

        // ── 5. FILTER ENVELOPE (AD) ─────────────────────────────────────────
        float effDecay = std::max(decayTime.value, minDecay.value);
        vcfEnv *= tau(effDecay);

        float accentC = tau(60.0f);
        accentVcf *= accentC;
        accentVca *= accentC;

        // ── 6. DYNAMIC CUTOFF ────────────────────────────────────────────────
        float dynamicCutoff = 0.85f * cutoff.value * 0.01f * envMod.value * 0.01f + 0.1f;

        // ── 7. RESONANCE ────────────────────────────────────────────────────
        float res = 0.90f * ((1.0f - Math::pow(1.0f - resonance.value * 0.01f, 2.0f)) + accentVcf * 0.15f);
        filter.setResonance(res);

        // ── 8. FILTER ───────────────────────────────────────────────────────
        filter.setCutoff(dynamicCutoff);
        filter.setSampleData(osc, 0);
        filter.setSampleData(filter.lp[0], 1);
        float sig = filter.lp[1];

        // float sig = osc;

        // ── 9. HP FILTER ────────────────────────────────────────────────────
        float hpCoeff = 0.0005f + hpCutoff.value * 0.0005f;
        hpState += hpCoeff * (sig - hpState);
        sig = (sig - hpState) * (1.0f + hpCutoff.value * 0.015f);

        // ── 10. AMP ENVELOPE ────────────────────────────────────────────────
        if (gateOpen) {
            ampEnv = 1.0f;
        } else {
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