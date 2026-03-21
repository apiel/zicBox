#pragma once

#include "audio/effects/applyBoost.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyReverb.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"
#include "helpers/clamp.h"

#include <algorithm>
#include <cmath>

// Merged hi-hat + clap engine.
//
// Character morphs between two fully independent synthesis layers:
//   0%   = pure hi-hat (inharmonic oscillator bank)
//  100%  = pure clap   (burst-noise + body)
//  In between: rimshots, shakers, snap hats, hand percussion.
//
// PAGE 1 — GLOBAL
//   0  Character    0=HiHat … 100=Clap
//   1  Duration     ms  (shared amp envelope length for both layers)
//   2  Boost        %   (bass boost / compression)
//   3  Reverb       %
//
// PAGE 2 — CLAP LAYER
//   4  Clap Bursts     count 1–10
//   5  Clap Spacing    % (time between bursts)
//   6  Clap Burst Dec  % (each burst's decay speed)
//   7  Clap Noise Clr  % (white→pink blend)
//   8  Clap Punch      % (transient shape)
//   9  Clap Transient  % (ultra-early click)
//
// PAGE 3 — HIHAT LAYER
//  10  Hi Inharmonic   % (partial spread)
//  11  Hi Detune       % (osc2 beating offset)
//  12  Hi FM Amt       % (inter-oscillator FM)
//  13  Hi Tone         % (square→triangle morph)
//  14  Hi Noise Mix    % (noise blend into metal)
//  15  Hi BP Freq      Hz (bandpass centre)
//  16  Hi BP Width     % (bandpass Q)
//  17  Hi Low Cut      % (HP to remove rumble)
//  18  Hi Tightness    % (amp envelope tail curve)
//  19  Hi Choke        % (sharpens decay)
//
// 20–23 free

class DrumHiClap23 : public EngineBase<DrumHiClap23> {

protected:
    const float sampleRate;

    float* reverbBuffer = nullptr;
    int reverbIndex = 0;

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
    static float pct(const Param& p) { return p.value * 0.01f; }

    void updateBiquad()
    {
        // Clap biquad bandpass sits at a fixed sweet-spot (~1.5–3 kHz)
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
    Param params[24] = {
        // ── PAGE 1: GLOBAL ────────────────────────────────────────────────────
        // 0
        { .label = "Character", .unit = "Hi-Clap", .value = 0.0f },
        // 1
        { .label = "Duration", .unit = "ms", .value = 80.0f, .min = 5.0f, .max = 2000.0f, .step = 5.0f },
        // 2
        { .label = "Boost", .unit = "%", .value = 0.0f, .min = -100.0f },
        // 3
        { .label = "Reverb", .unit = "%", .value = 0.0f },

        // ── PAGE 2: CLAP LAYER ────────────────────────────────────────────────
        // 4
        { .label = "Clap Bursts", .unit = "", .value = 5.0f, .min = 1.0f, .max = 10.0f, .step = 1.0f },
        // 5
        { .label = "Clap Spacing", .unit = "%", .value = 30.0f },
        // 6
        { .label = "Clap Bst Dec", .unit = "%", .value = 25.0f },
        // 7
        { .label = "Clap Noise", .unit = "%", .value = 70.0f, .onUpdate = [](void* ctx, float) { static_cast<DrumHiClap23*>(ctx)->updateBiquad(); } },
        // 8
        { .label = "Clap Punch", .unit = "%", .value = 50.0f },
        // 9
        { .label = "Clap Trans", .unit = "%", .value = 0.0f },

        // ── PAGE 3: HIHAT LAYER ───────────────────────────────────────────────
        // 10
        { .label = "Hi Inharmonic", .unit = "%", .value = 40.0f },
        // 11
        { .label = "Hi Detune", .unit = "%", .value = 20.0f },
        // 12
        { .label = "Hi FM Amt", .unit = "%", .value = 25.0f },
        // 13
        { .label = "Hi Tone", .unit = "%", .value = 50.0f },
        // 14
        { .label = "Hi Noise Mix", .unit = "%", .value = 20.0f },
        // 15
        { .label = "Hi BP Freq", .unit = "Hz", .value = 5000.0f, .min = 1000.0f, .max = 14000.0f, .step = 100.0f },
        // 16
        { .label = "Hi BP Width", .unit = "%", .value = 60.0f },
        // 17
        { .label = "Hi Low Cut", .unit = "%", .value = 50.0f },
        // 18
        { .label = "Hi Tightness", .unit = "%", .value = 50.0f },
        // 19
        { .label = "Hi Choke", .unit = "%", .value = 0.0f },

        // 20–23 free
        { .label = "-", .unit = "", .value = 0.0f },
        { .label = "-", .unit = "", .value = 0.0f },
        { .label = "-", .unit = "", .value = 0.0f },
        { .label = "-", .unit = "", .value = 0.0f },
    };

    // Convenience references
    Param& character = params[0];
    Param& duration = params[1];
    Param& boost = params[2];
    Param& reverb = params[3];

    Param& clapBursts = params[4];
    Param& clapSpacing = params[5];
    Param& clapBurstDec = params[6];
    Param& clapNoiseClr = params[7];
    Param& clapPunch = params[8];
    Param& clapTrans = params[9];

    Param& hiInharmonic = params[10];
    Param& hiDetune = params[11];
    Param& hiFmAmt = params[12];
    Param& hiTone = params[13];
    Param& hiNoiseMix = params[14];
    Param& hiBpFreq = params[15];
    Param& hiBpWidth = params[16];
    Param& hiLowCut = params[17];
    Param& hiTightness = params[18];
    Param& hiChoke = params[19];

    DrumHiClap23(const float sampleRate, float* rvBuffer)
        : EngineBase(Drum, "HiClap23", params)
        , sampleRate(sampleRate)
        , reverbBuffer(rvBuffer)
    {
        updateBiquad();
        init();
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
        if (ampEnv <= 0.0f) return applyRvb(0.0f);

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

            // Burst sequencer
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

            // Biquad bandpass (colours the noise into a clap spectrum)
            clapSig = applyBiquadBP(clapSig);

            // Punch: boosts the very early part of the transient
            float punchN = pct(clapPunch);
            if (clapTime < 0.02f) {
                float highpassed = clapSig - lpState;
                lpState += 0.01f * (clapSig - lpState);
                clapSig += highpassed * punchN * 2.0f;
            }

            // Transient: ultra-early noise spike (first ~1 ms)
            if (clapTime < 0.001f) {
                clapSig += Noise::sample() * pct(clapTrans) * 5.0f;
            }

            // Amp shape: simple linear decay from ampEnv
            clapSig *= currentAmp;
        }

        // ── BLEND ─────────────────────────────────────────────────────────────
        float sig = hatSig * (1.0f - charBlend) + clapSig * charBlend;

        // ── GLOBAL: BOOST + REVERB ────────────────────────────────────────────
        if (boost.value > 0.0f) {
            float amt = pct(boost);
            sig = applyBoost(sig, amt, boostPrevIn, boostPrevOut);
        }

        return applyRvb(sig * velocity);
    }

private:
    float applyRvb(float out)
    {
        return applyReverb(out, reverb.value * 0.01f, reverbBuffer, reverbIndex);
    }
};