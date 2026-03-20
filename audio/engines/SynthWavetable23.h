#pragma once

#include "audio/Wavetable.h"
#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/linearInterpolation.h"
#include "audio/utils/math.h"

#include <algorithm>
#include <cmath>
#include <cstring>

// Wavetable synth with FM operator and self-feedback.
//
// Oscillator          : Wavetable carrier + sub (one octave below, sine↔square).
//                       The carrier phase advances normally each sample. FM and
//                       feedback are applied as a phase *offset* at lookup time,
//                       leaving the running phase clean.
//
// FM operator         : Sine modulator at (carrier × FM Ratio) modulates the
//                       carrier's read phase. FM Depth decays from 1→0 over
//                       FM Decay ms (0 ms = static / no decay).
//                       When FM Depth = 0 there is zero modulation.
//
// Feedback            : Previous output sample fed back as additional phase offset.
//
// Filter              : SVF morphing LP↔HP.
//                       Cutoff: -100 (full LP) … 0 (bypass) … +100 (full HP).
//                       Filter Env: signed — magnitude = depth, sign = direction,
//                       magnitude also controls decay speed (100% ≈ 20 ms, 0% ≈ 4 s).
//
// Envelopes           : AMP: linear Attack / hold-while-gate / exponential Release.
//                       VCF: decay-only, driven by Filter Env param.
//
// Glide               : Portamento in frequency space.
//
// FX chain            : Delay → Reverb.
//
// Constructor buffers : dlBuf (DELAY_BUF_SIZE floats)
//                       rvBuf (REVERB_BUF_SIZE floats)

class SynthWavetable23 : public EngineBase<SynthWavetable23> {

public:
    static constexpr int DELAY_BUF_SIZE = 48000;
    static constexpr int REVERB_BUF_SIZE = 16384;

protected:
    const float sampleRate;
    const float sampleRateDiv;

    FilterSVF svfFilter;

    // ── Oscillator state ──────────────────────────────────────────────────────
    float phase = 0.0f; // carrier phase, runs 0..sampleCount continuously
    float subPhase = 0.0f;
    float lfoPhase = 0.0f;
    float fmPhase = 0.0f; // FM modulator phase [0, 1)
    float fbSample = 0.0f; // previous output sample for feedback

    // ── Voice state ───────────────────────────────────────────────────────────
    float currentFreq = 440.0f;
    float targetFreq = 440.0f;
    float velocity = 1.0f;
    bool gateOpen = false;

    // ── Envelopes ─────────────────────────────────────────────────────────────
    float fmEnv = 1.0f;
    float vcfEnv = 0.0f;
    float ampEnv = 0.0f;
    int ampStage = 0;
    float ampAttackRate = 0.0f;
    float ampRelRate = 0.0f;

    // ── Delay ─────────────────────────────────────────────────────────────────
    float* delayBuf = nullptr;
    int delayWrite = 0;
    float dlyFbSmooth = 0.0f;

    // ── Reverb (Schroeder: 4 comb + 3 AP) ────────────────────────────────────
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

    float tau(float ms) const
    {
        return (ms < 0.01f) ? 0.0f : Math::exp(-1.0f / (sampleRate * ms * 0.001f));
    }

    float linearRate(float ms) const
    {
        if (ms < 0.01f) return 1.0f;
        return 1.0f / (sampleRate * ms * 0.001f);
    }

    // Read wavetable at absolute sample position with linear interpolation.
    // pos is in [0, sampleCount), wraps automatically.
    float wtRead(float pos)
    {
        float sc = (float)wavetable.sampleCount;
        // Wrap into [0, sampleCount)
        pos = pos - std::floor(pos / sc) * sc;
        return linearInterpolationAbsolute(pos, wavetable.sampleCount, wavetable.samples());
    }

    // ── AMP envelope tick ─────────────────────────────────────────────────────
    float ampEnvTick()
    {
        switch (ampStage) {
        case 0:
            ampEnv = 0.0f;
            break;
        case 1:
            ampEnv += ampAttackRate;
            if (ampEnv >= 1.0f) {
                ampEnv = 1.0f;
                ampStage = gateOpen ? 2 : 3;
            }
            break;
        case 2:
            ampEnv = 1.0f;
            if (!gateOpen) ampStage = 3;
            break;
        case 3:
            ampEnv *= ampRelRate;
            if (ampEnv < 0.0001f) {
                ampEnv = 0.0f;
                ampStage = 0;
            }
            break;
        }
        return ampEnv;
    }

    // ── Morphing LP/HP filter ─────────────────────────────────────────────────
    float applyMorphFilter(float sig, float cutoffParam, float res)
    {
        float absC = std::abs(cutoffParam);
        if (absC < 0.5f) return sig;

        float normCutoff = CLAMP(0.01f + absC * 0.0098f, 0.01f, 0.99f);
        float normRes = CLAMP(res, 0.0f, 0.98f);
        float t = (cutoffParam > 0.0f) ? 1.0f : 0.0f; // 0=LP, 1=HP

        svfFilter.setCutoff(normCutoff);
        svfFilter.setResonance(normRes);
        auto out = svfFilter.process12(sig);
        float filtered = lerp(out.lp, out.hp, t);

        return lerp(sig, filtered, absC * 0.01f);
    }

    // ── Reverb ────────────────────────────────────────────────────────────────
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

    // ── Delay ─────────────────────────────────────────────────────────────────
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
        sig = reverbProcess(sig, reverbMix.value * 0.01f, reverbSize.value * 0.01f, reverbDamp.value * 0.01f);
        return sig;
    }

public:
    Wavetable wavetable;
    char wtName[64] = "---";

    Param params[24] = {
        // ── PAGE 1: OSCILLATOR ────────────────────────────────────────────────
        { .label = "Frequency", .unit = "Hz", .value = 440.0f, .min = 20.0f, .max = 2000.0f, .step = 0.5f },
        { .label = "Wavetable", .string = wtName, .value = 0.0f, .min = 0.0f, .max = 0.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
             auto* self = (SynthWavetable23*)ctx;
             int pos = (int)val;
             self->wavetable.open(pos, false);
             strncpy(self->wtName,
                 self->wavetable.fileBrowser.getFileWithoutExtension(pos).c_str(),
                 sizeof(self->wtName) - 1);
         } },
        { .label = "Morph", .unit = "%", .value = 0.0f },
        { .label = "LFO Morph", .unit = "%", .value = 0.0f },
        { .label = "LFO Pitch", .unit = "st", .value = 0.0f, .max = 12.0f, .step = 0.1f },
        { .label = "LFO Rate", .unit = "Hz", .value = 2.0f, .min = 0.05f, .max = 30.0f, .step = 0.05f },
        { .label = "Sub Mix", .unit = "%", .value = 0.0f },
        { .label = "Sub Wave", .unit = "Sin-Sq", .value = 0.0f },

        // ── PAGE 2: FM ────────────────────────────────────────────────────────
        { .label = "FM Ratio", .unit = "x", .value = 2.0f, .min = 0.5f, .max = 8.0f, .step = 0.01f },
        { .label = "FM Depth", .unit = "%", .value = 0.0f },
        { .label = "FM Decay", .unit = "ms", .value = 0.0f, .min = 0.0f, .max = 4000.0f, .step = 5.0f },
        { .label = "Feedback", .unit = "%", .value = 0.0f },

        // ── PAGE 3: FILTER + AMP ──────────────────────────────────────────────
        { .label = "Cutoff", .unit = "%", .value = -50.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Resonance", .unit = "%", .value = 25.0f },
        { .label = "Filter Env", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Attack", .unit = "ms", .value = 10.0f, .min = 1.0f, .max = 2000.0f, .step = 1.0f },
        { .label = "Release", .unit = "ms", .value = 400.0f, .min = 5.0f, .max = 4000.0f, .step = 5.0f },
        { .label = "Glide", .unit = "ms", .value = 0.0f, .max = 1000.0f, .step = 5.0f },

        // ── PAGE 4: FX ────────────────────────────────────────────────────────
        { .label = "Reverb Mix", .unit = "%", .value = 0.0f },
        { .label = "Rvb Size", .unit = "%", .value = 50.0f },
        { .label = "Rvb Damp", .unit = "%", .value = 50.0f },
        { .label = "Dly Mix", .unit = "%", .value = 0.0f },
        { .label = "Dly Time", .unit = "ms", .value = 125.0f, .min = 10.0f, .max = 1000.0f, .step = 5.0f },
        { .label = "Dly Fdbk", .unit = "%", .value = 0.0f },
    };

    Param& freq = params[0];
    Param& wtSelect = params[1];
    Param& morph = params[2];
    Param& lfoToMorph = params[3];
    Param& lfoToPitch = params[4];
    Param& lfoRate = params[5];
    Param& subMix = params[6];
    Param& subWave = params[7];
    Param& fmRatio = params[8];
    Param& fmDepth = params[9];
    Param& fmDecay = params[10];
    Param& feedback = params[11];
    Param& cutoff = params[12];
    Param& resonance = params[13];
    Param& filterEnv = params[14];
    Param& ampAttack = params[15];
    Param& ampRelease = params[16];
    Param& glide = params[17];
    Param& reverbMix = params[18];
    Param& reverbSize = params[19];
    Param& reverbDamp = params[20];
    Param& dlyMix = params[21];
    Param& dlyTime = params[22];
    Param& dlyFdbk = params[23];

    SynthWavetable23(float sr, float* dlBuf, float* rvBuf)
        : EngineBase(Synth, "Wavetable23", params)
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

        wtSelect.max = std::max(0.0f, (float)(wavetable.fileBrowser.count - 1));

        wavetable.open(0, true);
        strncpy(wtName,
            wavetable.fileBrowser.getFileWithoutExtension(0).c_str(),
            sizeof(wtName) - 1);

        init();
    }

    void noteOnImpl(uint8_t note, float vel)
    {
        velocity = vel;

        float noteOffset = static_cast<float>(note) - 60.0f;
        targetFreq = freq.value * std::pow(2.0f, noteOffset / 12.0f);

        if (!gateOpen || glide.value < 0.5f)
            currentFreq = targetFreq;

        gateOpen = true;
        phase = 0.0f;
        subPhase = 0.0f;
        fmPhase = 0.0f;
        fbSample = 0.0f;
        fmEnv = 1.0f;
        vcfEnv = 1.0f;

        ampAttackRate = linearRate(ampAttack.value);
        ampRelRate = tau(ampRelease.value);
        ampStage = 1;
    }

    void noteOffImpl(uint8_t)
    {
        gateOpen = false;
    }

    float sampleImpl()
    {
        if (ampStage == 0 && !gateOpen)
            return bufferedFxProcess(0.0f);

        // ── 1. GLIDE ──────────────────────────────────────────────────────────
        if (glide.value > 0.5f) {
            float c = tau(glide.value);
            currentFreq += (1.0f - c) * (targetFreq - currentFreq);
        } else {
            currentFreq = targetFreq;
        }

        // ── 2. LFO ────────────────────────────────────────────────────────────
        lfoPhase += lfoRate.value * sampleRateDiv;
        if (lfoPhase > 1.0f) lfoPhase -= 1.0f;
        float lfoOut = Math::fastSin(PI_X2 * lfoPhase);

        // ── 3. FREQUENCIES ────────────────────────────────────────────────────
        float pitchRatio = std::pow(2.0f, lfoOut * lfoToPitch.value / 12.0f);
        float carrierFreq = std::max(1.0f, currentFreq * pitchRatio);
        float sampleInc = carrierFreq * sampleRateDiv; // in [0,1) phase units per sample

        // ── 4. FM ENVELOPE ────────────────────────────────────────────────────
        if (fmDecay.value > 0.5f)
            fmEnv *= tau(fmDecay.value);
        // else fmEnv stays at 1.0 (static FM)

        // ── 5. FM MODULATOR ───────────────────────────────────────────────────
        float fmOffset = 0.0f;
        if (fmDepth.value > 0.001f) {
            float modFreq = carrierFreq * fmRatio.value;
            fmPhase += modFreq * sampleRateDiv;
            if (fmPhase > 1.0f) fmPhase -= 1.0f;
            float modSig = Math::fastSin(PI_X2 * fmPhase);
            // FM offset in samples (how far we shift the read head in the wavetable frame)
            // fmDepth 0–100% → index 0–4× sampleCount offset peak
            fmOffset = modSig * fmDepth.value * 0.01f * fmEnv * (float)wavetable.sampleCount * 4.0f;
        }

        // ── 6. FEEDBACK OFFSET ────────────────────────────────────────────────
        float fbOffset = 0.0f;
        if (feedback.value > 0.001f) {
            // feedback 0–100% → up to 0.5 × sampleCount offset
            fbOffset = fbSample * feedback.value * 0.01f * (float)wavetable.sampleCount * 0.5f;
        }

        // ── 7. CARRIER PHASE + MORPH ─────────────────────────────────────────
        phase += sampleInc * (float)wavetable.sampleCount;
        if (phase >= (float)wavetable.sampleCount) phase -= (float)wavetable.sampleCount;

        float morphPos = CLAMP(morph.value * 0.01f + lfoOut * lfoToMorph.value * 0.005f, 0.0f, 1.0f);
        wavetable.morph(morphPos);

        // ── 8. WAVETABLE LOOKUP with FM + feedback offset ─────────────────────
        float osc = wtRead(phase + fmOffset + fbOffset);

        // Store this sample for next-cycle feedback
        fbSample = osc;

        // ── 9. SUB-OSCILLATOR ─────────────────────────────────────────────────
        subPhase += (carrierFreq * 0.5f) * sampleRateDiv;
        if (subPhase > 1.0f) subPhase -= 1.0f;
        float subSine = Math::fastSin(PI_X2 * subPhase);
        float subSq = (subPhase < 0.5f) ? 1.0f : -1.0f;
        float sub = lerp(subSine, subSq, subWave.value * 0.01f);
        osc = lerp(osc, sub, subMix.value * 0.01f);

        // ── 10. FILTER ENVELOPE ───────────────────────────────────────────────
        float envAbs = std::abs(filterEnv.value);
        float decayMs = lerp(4000.0f, 20.0f, envAbs * 0.01f);
        vcfEnv *= tau(decayMs);

        float envSign = (filterEnv.value >= 0.0f) ? 1.0f : -1.0f;
        float dynCutoff = CLAMP(cutoff.value + envSign * vcfEnv * envAbs, -100.0f, 100.0f);

        // ── 11. FILTER ────────────────────────────────────────────────────────
        float res = CLAMP(
            0.90f * (1.0f - Math::pow(1.0f - resonance.value * 0.01f, 2.0f)),
            0.0f, 0.98f);

        osc = CLAMP(osc, -1.0f, 1.0f);
        float sig = applyMorphFilter(osc, dynCutoff, res);

        // ── 12. AMP ENVELOPE + VELOCITY ──────────────────────────────────────
        sig *= ampEnvTick() * velocity;

        // ── 13. BUFFERED FX ───────────────────────────────────────────────────
        return bufferedFxProcess(sig);
    }
};