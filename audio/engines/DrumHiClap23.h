#pragma once

#include "audio/effects/applyBoost.h"
#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"
#include "helpers/clamp.h"

#include <algorithm>
#include <cmath>

// Merged hi-hat + clap engine.
//
// Character morphs between two fully independent synthesis layers:
//   0%   = pure hi-hat (inharmonic oscillator bank)
//  100%  = pure clap   (burst-noise)
//  In between: rimshots, shakers, snap hats, hand percussion.
//
// PAGE 1 — GLOBAL
//   0  Duration        ms  (shared amp envelope length)
//   1  Character       0=HiHat … 100=Clap
//
// PAGE 2 — CLAP LAYER
//   2  Clap Bursts     count 1–10
//   3  Clap Spacing    %
//   4  Clap Burst Dec  %
//   5  Clap Noise Clr  %
//   6  Clap Punch      %
//   7  Clap Transient  %
//
// PAGE 3 — HIHAT LAYER
//   8  Hi Inharmonic   %
//   9  Hi Detune       %
//  10  Hi FM Amt       %
//  11  Hi Tone         %
//  12  Hi Noise Mix    %
//  13  Hi BP Freq      Hz
//  14  Hi BP Width     %
//  15  Hi Low Cut      %
//  16  Hi Tightness    %
//  17  Hi Choke        %
//
// PAGE 4 — GLOBAL FX
//  18  Cutoff          % (-100=LP … 0=bypass … +100=HP)
//  19  Resonance       %
//  20  Boost           %
//  21  Rvb Mix         %
//  22  Rvb Damp        %
//
//  22–23 free

class DrumHiClap23 : public EngineBase<DrumHiClap23> {

public:
    static constexpr int REVERB_BUF_SIZE = 16384;

protected:
    const float sampleRate;

    // ── Reverb (Schroeder: 4 comb + 3 AP, same as SynthFm23) ─────────────────
    float* reverbBuf = nullptr;

    static constexpr int COMB_LEN[4] = { 1559, 1617, 1685, 1751 };
    static constexpr int AP_LEN[3] = { 347, 113, 37 };

    int combOff[4] = {};
    int apOff[3] = {};
    int combIdx[8] = {};
    int apIdx[4] = {};
    float combFb[8] = {};

    // ── Morphing LP/HP filter (same as SynthFm23) ─────────────────────────────
    FilterSVF svfFilter;

    float velocity = 1.0f;

    // ── Shared amp envelope ───────────────────────────────────────────────────
    float ampEnv = 0.0f;
    float ampStep = 0.0f;

    // ── Hi-hat layer ──────────────────────────────────────────────────────────
    float osc1Phases[6] = {};
    float osc2Phases[6] = {};

    // Cascaded SVF bandpass
    float bp1Lp = 0.0f, bp1Bp = 0.0f;
    float bp2Lp = 0.0f, bp2Bp = 0.0f;

    // DC / low-cut one-pole
    float dcState = 0.0f;

    // ── Clap layer ────────────────────────────────────────────────────────────
    float burstTimer = 0.0f;
    int burstIndex = 0;
    float burstEnv = 0.0f;
    float pink = 0.0f;
    float clapTime = 0.0f;

    // Body sine
    float bodyPhase = 0.0f;
    float bodyEnv = 0.0f;

    // Biquad bandpass for clap
    float b0_a0 = 0.0f, b1_a0 = 0.0f, b2_a0 = 0.0f;
    float a1_a0 = 0.0f, a2_a0 = 0.0f;
    float gainComp = 1.0f;
    float bp_x1 = 0.0f, bp_x2 = 0.0f;
    float bp_y1 = 0.0f, bp_y2 = 0.0f;

    // Transient HP state
    float lpState = 0.0f;

    // Boost state
    float boostPrevIn = 0.0f;
    float boostPrevOut = 0.0f;

    // ── Helpers ───────────────────────────────────────────────────────────────
    static float lerp(float a, float b, float t) { return a + t * (b - a); }
    static float pct(const Param& p)
    {
        return p.value * 0.01f;
    }

    // Morphing LP/HP — identical to SynthFm23's applyMorphFilter
    // cutoffParam: -100 = full LP, 0 = bypass, +100 = full HP
    float applyMorphFilter(float sig, float cutoffParam, float res)
    {
        float absC = std::abs(cutoffParam);
        if (absC < 0.5f) return sig;

        float normCutoff = CLAMP(0.01f + absC * 0.0098f, 0.01f, 0.99f);
        float normRes = CLAMP(res, 0.0f, 0.98f);
        float t = CLAMP((cutoffParam + 100.0f) * 0.005f, 0.0f, 1.0f); // 0=LP, 1=HP

        svfFilter.setCutoff(normCutoff);
        svfFilter.setResonance(normRes);
        auto out = svfFilter.process12(sig);
        float filtered = lerp(out.lp, out.hp, t);

        return lerp(sig, filtered, absC * 0.01f);
    }

    // Schroeder reverb — identical to SynthFm23's reverbProcess
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

    void updateBiquad()
    {
        float f0 = 1500.0f + pct(clapNoiseClr) * 1500.0f;
        float Q = 0.8f;
        float omega = 2.0f * (float)M_PI * f0 / sampleRate;
        float s = Math::sin(omega);
        float c = Math::cos(omega);
        float alpha = s / (2.0f * Q);
        float a0 = 1.0f + alpha;
        float inv = 1.0f / a0;
        b0_a0 = alpha * inv;
        b1_a0 = 0.0f;
        b2_a0 = -alpha * inv;
        a1_a0 = (-2.0f * c) * inv;
        a2_a0 = (1.0f - alpha) * inv;
        gainComp = 1.0f + Q;
    }

    float applyBiquadBP(float in)
    {
        float y = b0_a0 * in + b1_a0 * bp_x1 + b2_a0 * bp_x2
            - a1_a0 * bp_y1 - a2_a0 * bp_y2;
        bp_x2 = bp_x1;
        bp_x1 = in;
        bp_y2 = bp_y1;
        bp_y1 = y;
        return y * gainComp;
    }

public:
    enum ParamGroup {
        NONE,
        CLAP,
        HIHAT,
        FX,
    };

    Param params[23];
    Param& duration = addParam({ .label = "Duration", .unit = "ms", .value = 80.0f, .min = 5.0f, .max = 2000.0f, .step = 5.0f });
    Param& character = addParam({ .label = "Character", .unit = "Hi-Clap", .value = 0.0f });

    Param& clapBursts = addParam({ .label = "Clap Bursts", .unit = "", .value = 5.0f, .min = 1.0f, .max = 10.0f, .step = 1.0f, .group = CLAP });
    Param& clapSpacing = addParam({ .label = "Clap Spacing", .unit = "%", .value = 30.0f, .group = CLAP });
    Param& clapBurstDec = addParam({ .label = "Clap Bst Dec", .unit = "%", .value = 25.0f, .group = CLAP });
    Param& clapNoiseClr = addParam({ .label = "Clap Noise", .unit = "%", .value = 70.0f, .group = CLAP, .onUpdate = [](void* ctx, float) { static_cast<DrumHiClap23*>(ctx)->updateBiquad(); } }, false);
    Param& clapPunch = addParam({ .label = "Clap Punch", .unit = "%", .value = 50.0f, .group = CLAP });
    Param& clapTrans = addParam({ .label = "Clap Trans", .unit = "%", .value = 0.0f, .group = CLAP });

    Param& hiInharmonic = addParam({ .label = "Hi Inharmonic", .unit = "%", .value = 40.0f, .group = HIHAT });
    Param& hiDetune = addParam({ .label = "Hi Detune", .unit = "%", .value = 20.0f, .group = HIHAT });
    Param& hiFmAmt = addParam({ .label = "Hi FM Amt", .unit = "%", .value = 25.0f, .group = HIHAT });
    Param& hiTone = addParam({ .label = "Hi Tone", .unit = "%", .value = 50.0f, .group = HIHAT });
    Param& hiNoiseMix = addParam({ .label = "Hi Noise Mix", .unit = "%", .value = 20.0f, .group = HIHAT });
    Param& hiBpFreq = addParam({ .label = "Hi BP Freq", .unit = "Hz", .value = 5000.0f, .min = 1000.0f, .max = 14000.0f, .step = 100.0f, .group = HIHAT });
    Param& hiBpWidth = addParam({ .label = "Hi BP Width", .unit = "%", .value = 60.0f, .group = HIHAT });
    Param& hiLowCut = addParam({ .label = "Hi Low Cut", .unit = "%", .value = 50.0f, .group = HIHAT });
    Param& hiTightness = addParam({ .label = "Hi Tightness", .unit = "%", .value = 50.0f, .group = HIHAT });
    Param& hiChoke = addParam({ .label = "Hi Choke", .unit = "%", .value = 0.0f, .group = HIHAT });

    Param& cutoff = addParam({ .label = "Cutoff", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .group = FX });
    Param& resonance = addParam({ .label = "Resonance", .unit = "%", .value = 0.0f, .group = FX });
    Param& boost = addParam({ .label = "Boost", .unit = "%", .value = 0.0f, .min = -100.0f, .group = FX });
    Param& reverbMix = addParam({ .label = "Rvb Mix", .unit = "%", .value = 0.0f, .group = FX });
    Param& reverbDamp = addParam({ .label = "Rvb Damp", .unit = "%", .value = 50.0f, .group = FX });

    DrumHiClap23(const float sampleRate, float* rvBuf)
        : EngineBase(Drum, "HiClap23", params)
        , sampleRate(sampleRate)
        , reverbBuf(rvBuf)
    {
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

        updateBiquad();

        init(); // Init because of clap noise
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        ampEnv = 1.0f;

        float durSamples = std::max(1.0f, sampleRate * (duration.value * 0.001f));
        ampStep = 1.0f / durSamples;

        // Reset hi-hat state
        bp1Lp = bp1Bp = bp2Lp = bp2Bp = dcState = 0.0f;
        for (int i = 0; i < 6; ++i) {
            osc1Phases[i] = 0.0f;
            osc2Phases[i] = 0.0f;
        }

        // Reset clap state
        burstTimer = 0.0f;
        burstIndex = 0;
        burstEnv = 1.0f;
        pink = 0.0f;
        clapTime = 0.0f;
        bodyPhase = 0.0f;
        bodyEnv = 1.0f;
        bp_x1 = bp_x2 = bp_y1 = bp_y2 = 0.0f;
        lpState = 0.0f;
        boostPrevIn = boostPrevOut = 0.0f;
    }

    float sampleImpl()
    {
        if (ampEnv <= 0.0f) return reverbProcess(0.0f, reverbMix.value * 0.01f, 0.5f, reverbDamp.value * 0.01f);

        float currentAmp = ampEnv;
        ampEnv -= ampStep;

        float charBlend = pct(character); // 0=hat, 1=clap

        // ── HIHAT LAYER ───────────────────────────────────────────────────────
        float hatSig = 0.0f;
        if (charBlend < 0.999f) {
            static const float ratios[6] = { 1.0f, 1.413f, 1.854f, 2.278f, 3.014f, 4.127f };
            float metalFreq = 3500.0f;
            float inhSpread = hiInharmonic.value * 12.0f;
            float detuneHz = metalFreq * hiDetune.value * 0.0005f;
            float fmStrength = hiFmAmt.value * 0.04f;
            float toneBlend = pct(hiTone);

            float metalOut1 = 0.0f, metalOut2 = 0.0f;
            float lastSig1 = 0.0f, lastSig2 = 0.0f;

            for (int i = 0; i < 6; ++i) {
                float freq1 = metalFreq * ratios[i] + i * inhSpread;
                freq1 = std::min(freq1, sampleRate * 0.47f);
                osc1Phases[i] += freq1 / sampleRate + lastSig1 * fmStrength;
                if (osc1Phases[i] > 1.0f) osc1Phases[i] -= 1.0f;
                float sq1 = osc1Phases[i] > 0.5f ? 1.0f : -1.0f;
                float tri1 = 2.0f * std::abs(2.0f * (osc1Phases[i] - std::floor(osc1Phases[i] + 0.5f))) - 1.0f;
                float s1 = sq1 + (tri1 - sq1) * toneBlend;
                lastSig1 = s1;
                metalOut1 += (i % 2 == 0) ? s1 : -s1 * 0.8f;

                float freq2 = (metalFreq + detuneHz) * ratios[i] + i * inhSpread;
                freq2 = std::min(freq2, sampleRate * 0.47f);
                osc2Phases[i] += freq2 / sampleRate + lastSig2 * fmStrength;
                if (osc2Phases[i] > 1.0f) osc2Phases[i] -= 1.0f;
                float sq2 = osc2Phases[i] > 0.5f ? 1.0f : -1.0f;
                float tri2 = 2.0f * std::abs(2.0f * (osc2Phases[i] - std::floor(osc2Phases[i] + 0.5f))) - 1.0f;
                float s2 = sq2 + (tri2 - sq2) * toneBlend;
                lastSig2 = s2;
                metalOut2 += (i % 2 == 0) ? s2 : -s2 * 0.8f;
            }

            hatSig = (metalOut1 + metalOut2) * (1.0f / 12.0f);

            // Noise blend into metal
            hatSig = hatSig * (1.0f - pct(hiNoiseMix)) + Noise::sample() * pct(hiNoiseMix);

            // Cascaded SVF bandpass
            {
                float q = 0.3f + hiBpWidth.value * 0.008f;
                float fb = 1.0f / q;
                float f1 = std::min(2.0f * Math::sin((float)M_PI * hiBpFreq.value / sampleRate), 0.49f);
                float hp1 = hatSig - bp1Bp * fb - bp1Lp;
                bp1Bp = std::clamp(bp1Bp + f1 * hp1, -4.0f, 4.0f);
                bp1Lp = std::clamp(bp1Lp + f1 * bp1Bp, -4.0f, 4.0f);
                float f2 = std::min(2.0f * Math::sin((float)M_PI * hiBpFreq.value * 1.22f / sampleRate), 0.49f);
                float hp2 = hatSig - bp2Bp * fb - bp2Lp;
                bp2Bp = std::clamp(bp2Bp + f2 * hp2, -4.0f, 4.0f);
                bp2Lp = std::clamp(bp2Lp + f2 * bp2Bp, -4.0f, 4.0f);
                float wet = 0.4f + hiBpWidth.value * 0.005f;
                hatSig = hatSig * (1.0f - wet) + (bp1Bp + bp2Bp) * wet * 0.5f;
            }

            // Low cut
            {
                float hpCoeff = 0.001f + hiLowCut.value * 0.009f;
                dcState += hpCoeff * (hatSig - dcState);
                hatSig = (hatSig - dcState) * (1.0f + hiLowCut.value * 0.1f);
            }

            // Tightness + choke shaping
            float chokeShape = 1.0f + hiChoke.value * 0.06f;
            float shapedAmp = Math::pow(currentAmp, chokeShape);
            float tightFactor = Math::pow(shapedAmp, 1.0f + hiTightness.value * 0.03f);
            hatSig *= tightFactor;
        }

        // ── CLAP LAYER ────────────────────────────────────────────────────────
        float clapSig = 0.0f;
        if (charBlend > 0.001f) {
            clapTime += 1.0f / sampleRate;
            float spacingSec = pct(clapSpacing) * 0.03f + 0.01f;
            float decayTimeSec = pct(clapBurstDec) * 0.3f + 0.02f;

            if (burstIndex < (int)clapBursts.value) {
                burstTimer += 1.0f / sampleRate;
                if (burstTimer >= spacingSec) {
                    burstTimer -= spacingSec;
                    burstIndex++;
                    burstEnv = 1.0f;
                }
            }

            if (burstEnv > 0.0f) {
                float white = Noise::sample();
                pink = 0.98f * pink + 0.02f * white;
                float noise = pink * (1.0f - pct(clapNoiseClr)) + white * pct(clapNoiseClr);
                clapSig += noise * burstEnv;
                burstEnv *= Math::exp(-1.0f / (sampleRate * decayTimeSec));
            }

            clapSig = applyBiquadBP(clapSig);

            // Punch
            float punchN = pct(clapPunch);
            if (clapTime < 0.02f) {
                float highpassed = clapSig - lpState;
                lpState += 0.01f * (clapSig - lpState);
                clapSig += highpassed * punchN * 2.0f;
            }

            // Transient click
            if (clapTime < 0.001f)
                clapSig += Noise::sample() * pct(clapTrans) * 5.0f;

            clapSig *= currentAmp;
        }

        // ── BLEND ─────────────────────────────────────────────────────────────
        float sig = hatSig * (1.0f - charBlend) + clapSig * charBlend;

        // ── GLOBAL FX ─────────────────────────────────────────────────────────
        // Morphing LP/HP filter (same as SynthFm23)
        sig = CLAMP(sig, -1.0f, 1.0f);
        sig = applyMorphFilter(sig, cutoff.value, resonance.value * 0.01f);

        // Boost
        if (boost.value > 0.0f)
            sig = applyBoost(sig, pct(boost), boostPrevIn, boostPrevOut);

        // Schroeder reverb (same as SynthFm23)
        sig = reverbProcess(sig * velocity, reverbMix.value * 0.01f, 0.5f, reverbDamp.value * 0.01f);

        return sig;
    }
};