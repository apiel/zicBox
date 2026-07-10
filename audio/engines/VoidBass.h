#pragma once

#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"
#include <algorithm>
#include <cmath>

class VoidBass : public EngineBase<VoidBass> {
public:
    static constexpr int DELAY_BUF_SIZE = 48000;

protected:
    const float sampleRate;
    const float sampleRateDiv;

    // Oscillator States
    float phase = 0.0f;
    float subPhase = 0.0f;
    float currentFreq = 110.0f;
    float targetFreq = 110.0f;
    float lfoPhase = 0.0f;

    // Filter States (4-Pole Non-linear Ladder)
    float stage[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float oldStage[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    // Envelope & Mod States
    bool gateOpen = false;
    bool accented = false;
    float velocity = 1.0f;
    float vcfEnv = 0.0f;
    float ampEnv = 0.0f;
    float accentVcf = 0.0f;
    float accentVca = 0.0f;
    float hpState = 0.0f;

    // FX Buffers & Pointers
    float* delayBuf = nullptr;
    int delayWrite = 0;
    float dlyFbSmooth = 0.0f;

    static float lerp(float a, float b, float t) { return a + t * (b - a); }

    float tau(float ms) const {
        return (ms < 0.01f) ? 0.0f : std::exp(-1.0f / (sampleRate * ms * 0.001f));
    }

    // --- Elite Multi-Stage Wave Morph Engine ---
    float generateMorphOsc(float p, float morphVal, float pwmVal) {
        // Map 0.0-1.0 morph into 3 distinct zones
        // Zone 1 (0.0->0.33): Sine to Triangle
        // Zone 2 (0.33->0.66): Triangle to Saw
        // Zone 3 (0.66->1.0): Saw to Square/PWM
        float oscOut = 0.0f;

        if (morphVal < 0.333f) {
            float t = morphVal * 3.0f;
            float sine = Math::fastSin(PI_X2 * p);
            float tri = 1.0f - 4.0f * std::abs(std::remainder(p, 1.0f) - 0.5f);
            oscOut = lerp(sine, tri, t);
        } else if (morphVal < 0.666f) {
            float t = (morphVal - 0.333f) * 3.0f;
            float tri = 1.0f - 4.0f * std::abs(std::remainder(p, 1.0f) - 0.5f);
            float saw = 2.0f * p - 1.0f;
            oscOut = lerp(tri, saw, t);
        } else {
            float t = (morphVal - 0.666f) * 3.0f;
            float saw = 2.0f * p - 1.0f;
            float targetPW = lerp(0.5f, pwmVal, t);
            float sq = (p < targetPW) ? 1.0f : -1.0f;
            oscOut = lerp(saw, sq, t);
        }
        return oscOut;
    }

    // --- OP-Z Inspired "Safe Wild" Rubber Ladder Filter ---
    float processVoidFilter(float input, float cutoffPct, float resPct) {
        // Map parameters cleanly using basic analog approximations
        float cutoffHz = 20.0f * std::pow(1000.0f, cutoffPct);
        float g = std::tan(static_cast<float>(M_PI) * cutoffHz * sampleRateDiv);
        
        // Dynamic resonance compression curve: safe limits even at 100%
        float maxRes = 3.95f; 
        float r = resPct * maxRes;

        // Auto-gain compensation to keep the fundamental bass heavy when resonance screams
        float compensation = resPct * 0.5f; 

        // 4-Pole non-linear solving loop using standard low-pass ladder topologies
        float h0 = 1.0f / (1.0f + g);
        
        // Feed forward paths + dynamic saturation within the feedback loop
        for (int sampleLoop = 0; sampleLoop < 2; sampleLoop++) { // 2x internal oversampling step
            float feedback = stage[3] - (input * compensation);
            float saturatedFeedback = std::tanh(feedback * r);

            float I = input - saturatedFeedback;

            // Transistor ladder stages
            stage[0] = (I * g + oldStage[0]) * h0;
            stage[1] = (stage[0] * g + oldStage[1]) * h0;
            stage[2] = (stage[1] * g + oldStage[2]) * h0;
            stage[3] = (stage[2] * g + oldStage[3]) * h0;

            for (int i = 0; i < 4; i++) oldStage[i] = stage[i];
        }

        return stage[3];
    }

    // --- Spatialization & Delay Core Processors ---
    float delayProcess(float sig, float mix, float timeMs, float fdbk) {
        if (mix < 0.001f) return sig;
        int delaySmp = std::max(1, std::min((int)(timeMs * 0.001f * sampleRate), DELAY_BUF_SIZE - 1));
        int readIdx = (delayWrite - delaySmp + DELAY_BUF_SIZE) % DELAY_BUF_SIZE;
        float delayed = delayBuf[readIdx];

        float fbTarget = fdbk * 0.01f * 0.85f;
        dlyFbSmooth += 0.001f * (fbTarget - dlyFbSmooth);

        delayBuf[delayWrite] = sig + delayed * dlyFbSmooth;
        delayWrite = (delayWrite + 1) % DELAY_BUF_SIZE;

        return lerp(sig, sig + delayed * 0.7f, mix * 0.01f);
    }

public:
    Param params[18] = {
        { .label = "Frequency", .unit = "Hz", .value = 130.81f, .min = 20.0f, .max = 800.0f, .step = 0.1f },
        { .label = "Morph", .unit = "0-100", .value = 0.0f },
        { .label = "Pulse Width", .unit = "%", .value = 50.0f, .min = 5.0f, .max = 95.0f },
        { .label = "Wavefold", .unit = "%", .value = 0.0f },
        { .label = "Sub Mix", .unit = "%", .value = 0.0f },
        { .label = "Cutoff", .unit = "%", .value = 60.0f },
        { .label = "Resonance", .unit = "%", .value = 20.0f },
        { .label = "Env Mod", .unit = "%", .value = 40.0f },
        { .label = "Decay", .unit = "ms", .value = 250.0f, .min = 10.0f, .max = 2000.0f, .step = 5.0f },
        { .label = "Accent", .unit = "%", .value = 50.0f },
        { .label = "HP Cut", .unit = "%", .value = 10.0f },
        { .label = "LFO Rate", .unit = "Hz", .value = 1.5f, .min = 0.05f, .max = 30.0f, .step = 0.05f },
        { .label = "LFO Target", .unit = "Dst", .value = 0.0f, .min = 0.0f, .max = 2.0f, .step = 1.0f }, // 0:Off, 1:Morph, 2:Pitch
        { .label = "Glide", .unit = "ms", .value = 0.0f, .max = 1000.0f, .step = 5.0f },
        { .label = "Drive", .unit = "%", .value = 10.0f },
        { .label = "Dly Mix", .unit = "%", .value = 0.0f },
        { .label = "Dly Time", .unit = "ms", .value = 250.0f, .min = 10.0f, .max = 1000.0f, .step = 5.0f },
        { .label = "Dly Fdbk", .unit = "%", .value = 30.0f },
    };

    // Fast-binding parameters mapped explicitly
    Param& freq = params[0]; Param& morph = params[1]; Param& pw = params[2];
    Param& wavefold = params[3]; Param& subMix = params[4]; Param& cutoff = params[5];
    Param& resonance = params[6]; Param& envMod = params[7]; Param& decayTime = params[8];
    Param& accentAmt = params[9]; Param& hpCutoff = params[10]; Param& lfoRate = params[11];
    Param& lfoTarget = params[12]; Param& glide = params[13]; Param& drive = params[14];
    Param& dlyMix = params[15]; Param& dlyTime = params[16]; Param& dlyFdbk = params[17];

    VoidBass(float sr, float* dlBuf)
        : EngineBase(Synth, "VoidBass", params)
        , sampleRate(sr)
        , sampleRateDiv(1.0f / sr)
        , delayBuf(dlBuf)
    {
        init();
    }

    void noteOnImpl(uint8_t note, float vel) {
        velocity = vel;
        accented = (vel > 0.75f);
        float noteOffset = static_cast<float>(note) - 60.0f;
        targetFreq = freq.value * std::pow(2.0f, noteOffset / 12.0f);

        if (!gateOpen || glide.value < 0.5f) currentFreq = targetFreq;

        gateOpen = true;
        vcfEnv = 1.0f;
        ampEnv = 1.0f;

        if (accented) {
            float a = accentAmt.value * 0.01f;
            accentVcf = a * 1.0f;
            accentVca = a * 0.4f;
        } else {
            accentVcf = accentVca = 0.0f;
        }
    }

    void noteOffImpl(uint8_t) { gateOpen = false; }

    float sampleImpl() {
        if (ampEnv < 0.0001f && !gateOpen) return delayProcess(0.0f, dlyMix.value, dlyTime.value, dlyFdbk.value);

        // 1. Frequency Glide Execution
        if (glide.value > 0.5f) {
            currentFreq += (1.0f - tau(glide.value)) * (targetFreq - currentFreq);
        } else {
            currentFreq = targetFreq;
        }

        // 2. Modulating LFO Runtime
        lfoPhase += lfoRate.value * sampleRateDiv;
        if (lfoPhase > 1.0f) lfoPhase -= 1.0f;
        float lfoOut = Math::fastSin(PI_X2 * lfoPhase);

        // 3. Dynamic Modulation Targeting
        float targetOscFreq = currentFreq;
        float targetMorph = morph.value * 0.01f;
        if ((int)lfoTarget.value == 1) targetMorph = std::clamp(targetMorph + lfoOut * 0.2f, 0.0f, 1.0f);
        if ((int)lfoTarget.value == 2) targetOscFreq += (lfoOut * (currentFreq * 0.05f));

        // 4. Core Oscillator Generation
        phase += targetOscFreq * sampleRateDiv;
        if (phase > 1.0f) phase -= 1.0f;

        float osc = generateMorphOsc(phase, targetMorph, pw.value * 0.01f);

        // Sub-Oscillator (Square wave running exactly one octave below)
        subPhase += (currentFreq * 0.5f) * sampleRateDiv;
        if (subPhase > 1.0f) subPhase -= 1.0f;
        float subOsc = (subPhase < 0.5f) ? 0.7f : -0.7f;
        osc = lerp(osc, subOsc, subMix.value * 0.01f);

        // 5. Pre-Filter Wavefolder Saturation
        if (wavefold.value > 0.01f) {
            float foldAmt = wavefold.value * 0.01f * 3.0f;
            osc = std::sin(osc * (1.0f + foldAmt));
        }

        // 6. Time Envelopes Runtime
        vcfEnv *= tau(decayTime.value);
        float decayMod = tau(accented ? decayTime.value * 0.5f : decayTime.value);
        accentVcf *= decayMod;
        accentVca *= decayMod;

        // 7. Elite Lowpass Ladder Filtering
        float dynamicCutoff = (cutoff.value * 0.01f) + (vcfEnv * envMod.value * 0.01f) + (accentVcf * 0.3f);
        dynamicCutoff = std::clamp(dynamicCutoff, 0.01f, 0.95f);
        float dynamicRes = std::clamp((resonance.value * 0.01f) + (accentVcf * 0.1f), 0.0f, 0.99f);

        float sig = processVoidFilter(osc, dynamicCutoff, dynamicRes);

        // 8. Post Distortion Shaper
        if (drive.value > 0.01f) {
            float driveGain = 1.0f + (drive.value * 0.01f * 5.0f);
            sig = std::tanh(sig * driveGain);
        }

        // 9. Clean Sub-Bass DC/Highpass Stage
        float hpCoeff = 0.001f + (hpCutoff.value * 0.002f);
        hpState += hpCoeff * (sig - hpState);
        sig = sig - hpState;

        // 10. Amp Release Envelope Stage
        if (gateOpen) {
            ampEnv = 1.0f;
        } else {
            ampEnv *= tau(decayTime.value * (accented ? 0.7f : 1.3f));
        }

        sig *= ampEnv * (velocity + accentVca);

        // 11. Spatial FX Chain Processing
        sig = delayProcess(sig, dlyMix.value, dlyTime.value, dlyFdbk.value);

        return sig;
    }
};