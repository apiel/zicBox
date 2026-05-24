#pragma once

#include "audio/effects/applyDrive.h"
#include "audio/effects/applyReverb.h"
#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"
#include "helpers/clamp.h"

#include <algorithm>
#include <cmath>

class DrumGeneric : public EngineBase<DrumGeneric> {

public:
    static constexpr int REVERB_BUF_SIZE = 16384;

protected:
    const float sampleRate;

    float* reverbBuf = nullptr;
    int reverbIndex = 0;

    // snare
    float tonalPhase = 0.0f;
    float ringPhase = 0.0f;
    float bodyEnvelope = 0.0f;

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
    enum ParamTarget {
        NONE,
        CLAP,
        HIHAT,
        FX,
    };

    Param params[24];

    Param& bodyDuration = addParam({ .key = "bodyDuration", .label = "Body Len", .unit = "ms", .value = 400.0f, .min = 10.0f, .max = 2000.0f, .step = 10.0f });
    Param& baseFrequency = addParam({ .key = "baseFrequency", .label = "Body Freq", .unit = "Hz", .value = 100.0f, .min = 100.0f, .max = 400.0f });
    Param& bodyDecay = addParam({ .key = "bodyDecay", .label = "Body", .unit = "%", .value = 30.0f });
    Param& ringAmount = addParam({ .key = "ringAmount", .label = "Body Ring", .unit = "%", .value = 0.0f });
    Param& bodyShape = addParam({ .key = "bodyShape", .label = "Body Shape", .unit = "%", .value = 0.0f });
    Param& bodyBend = addParam({ .key = "bodyBend", .label = "Body Bend", .unit = "%", .value = 25.0f });
    Param& bendShape = addParam({ .key = "bendShape", .label = "Bend Shape", .unit = "%", .value = 0.0f });

    Param& character = addParam({ .key = "character", .label = "Hi / Clap", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f });
    Param& hiClapDuration = addParam({ .key = "hiClapDuration", .label = "Duration", .unit = "ms", .value = 80.0f, .min = 5.0f, .max = 2000.0f, .step = 5.0f });

    Param& clapNoiseClr = addParam({ .key = "clapNoiseClr", .label = "Clap Noise", .unit = "%", .value = 70.0f, .target = CLAP, .onUpdate = [](void* ctx, float) { static_cast<DrumGeneric*>(ctx)->updateBiquad(); } }, false);
    Param& clapPunch = addParam({ .key = "clapPunch", .label = "Clap Punch", .unit = "%", .value = 50.0f, .target = CLAP });
    Param& clapTrans = addParam({ .key = "clapTrans", .label = "Clap Trans", .unit = "%", .value = 0.0f, .target = CLAP });

    Param& hiInharmonic = addParam({ .key = "hiInharmonic", .label = "Hi Inharmonic", .unit = "%", .value = 40.0f, .target = HIHAT });
    Param& hiDetune = addParam({ .key = "hiDetune", .label = "Hi Detune", .unit = "%", .value = 20.0f, .target = HIHAT });
    Param& hiFmAmt = addParam({ .key = "hiFmAmt", .label = "Hi FM Amt", .unit = "%", .value = 25.0f, .target = HIHAT });
    Param& hiTone = addParam({ .key = "hiTone", .label = "Hi Tone", .unit = "%", .value = 50.0f, .target = HIHAT });
    Param& hiNoiseMix = addParam({ .key = "hiNoiseMix", .label = "Hi Noise Mix", .unit = "%", .value = 20.0f, .target = HIHAT });
    Param& hiBpFreq = addParam({ .key = "hiBpFreq", .label = "Hi BP Freq", .unit = "Hz", .value = 5000.0f, .min = 1000.0f, .max = 14000.0f, .step = 100.0f, .target = HIHAT });
    Param& hiBpWidth = addParam({ .key = "hiBpWidth", .label = "Hi BP Width", .unit = "%", .value = 60.0f, .target = HIHAT });
    Param& hiTightness = addParam({ .key = "hiTightness", .label = "Hi Tightness", .unit = "%", .value = 50.0f, .target = HIHAT });

    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .target = FX });
    Param& resonance = addParam({ .key = "resonance", .label = "Resonance", .unit = "%", .value = 0.0f, .target = FX });
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 15.0f });
    Param& reverb = addParam({ .key = "rvbMix", .label = "Reverb", .unit = "%", .value = 0.0f, .target = FX });

    DrumGeneric(const float sampleRate, float* rvBuffer)
        : EngineBase(Drum, "Drum", params)
        , sampleRate(sampleRate)
        , reverbBuf(rvBuffer)
    {
        if (reverbBuf) {
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

        float durSamples = std::max(1.0f, sampleRate * (hiClapDuration.value * 0.001f));
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

        tonalPhase = 0.0f;
        ringPhase = 0.0f;
        bodyEnvelope = 1.0f;
    }

    float sampleImpl()
    {
        if (ampEnv <= 0.0f) return applyRvb(0.0f);

        float currentAmp = ampEnv;
        ampEnv -= ampStep;

        // Tonal Part
        float bodyTime = 0.005f + (bodyDecay.value * 0.002f);
        bodyEnvelope *= Math::exp(-1.0f / (sampleRate * bodyTime));

        float bendDepth = bodyBend.value * 5.0f;
        float exponent = 1.0f + bendShape.value * 0.06f;
        float shapedEnv = Math::pow(bodyEnvelope, exponent);
        float fundFreq = baseFrequency.value + (shapedEnv * bendDepth);

        tonalPhase += fundFreq / sampleRate;
        if (tonalPhase > 1.0f) tonalPhase -= 1.0f;

        float fundamental = Math::sin(PI_X2 * tonalPhase);

        if (bodyShape.value > 0.0f) {
            fundamental = CLAMP(fundamental * bodyShape.value * 0.02f, -1.0f, 1.0f);
            fundamental *= (1.0f - (bodyShape.value * 0.003f));
        }

        ringPhase += (fundFreq * 1.61f) / sampleRate;
        if (ringPhase > 1.0f) ringPhase -= 1.0f;
        float ring = Math::sin(PI_X2 * ringPhase) * ringAmount.value * 0.01f;

        float tonalPart = (fundamental + ring) * bodyEnvelope;

        // Hi Clap
        float charBlend = (character.value + 100.0f) * 0.005f;

        // ── HIHAT LAYER ───────────────────────────────────────────────────────
        float hatSig = 0.0f;
        if (charBlend < 0.999f) {
            static const float ratios[6] = { 1.0f, 1.413f, 1.854f, 2.278f, 3.014f, 4.127f };
            float metalFreq = 3500.0f;
            float inhSpread = hiInharmonic.value * 12.0f;
            float detuneHz = metalFreq * hiDetune.value * 0.0005f;
            float fmStrength = hiFmAmt.value * 0.004f;
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

            // Tightness + choke shaping
            float tightFactor = Math::pow(currentAmp, 1.0f + hiTightness.value * 0.03f);
            hatSig *= tightFactor;
        }

        // ── CLAP LAYER ────────────────────────────────────────────────────────
        float clapSig = 0.0f;
        if (charBlend > 0.001f) {
            clapTime += 1.0f / sampleRate;
            float spacingSec = 1.5f;
            float decayTimeSec = 1.5f;

            if (burstIndex < 5) { // 5 bursts
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
        float hiClapSig = hatSig * (1.0f - charBlend) + clapSig * charBlend;

        float sig = (hiClapSig + tonalPart) * 0.5f;


        // ── GLOBAL FX ─────────────────────────────────────────────────────────
        // Morphing LP/HP filter (same as SynthFm23)
        sig = CLAMP(sig, -1.0f, 1.0f);
        sig = applyMorphFilter(sig, cutoff.value, resonance.value * 0.01f);

        if (drive.value > 0.0f) {
            sig = applyDrive(sig, pct(drive) * 4.0f);
        }

        sig = applyRvb(sig * velocity);

        return sig;
    }

    float applyRvb(float out)
    {
        return applyReverb(out, reverb.value * 0.01f, reverbBuf, reverbIndex);
    }
};