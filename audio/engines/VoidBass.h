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

    // Filter States (Cascaded Dual 4-Pole Non-linear Ladder for 48dB/oct Slope)
    float stageA[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float oldStageA[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float stageB[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float oldStageB[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

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

    float tau(float ms) const
    {
        return (ms < 0.01f) ? 0.0f : std::exp(-1.0f / (sampleRate * ms * 0.001f));
    }

    // --- Elite Multi-Stage Wave Morph Engine ---
    float generateMorphOsc(float p, float morphVal, float pwmVal)
    {
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

    // --- High-Punch Cascaded Dual-Ladder Filter (48 dB/oct) ---
    float processVoidFilter(float input, float cutoffPct, float resPct)
    {
        float cutoffHz = 20.0f * std::pow(1000.0f, cutoffPct);
        float g = std::tan(static_cast<float>(M_PI) * cutoffHz * sampleRateDiv);

        float maxRes = 3.95f;
        float r = resPct * maxRes;

        // Enhanced gain compensation to keep the low-end deep under extreme slopes
        float compensation = resPct * 0.65f;
        float h0 = 1.0f / (1.0f + g);

        for (int sampleLoop = 0; sampleLoop < 2; sampleLoop++) {
            // Cross-coupled feedback taken from the final output of the combined engine (stageB[3])
            float feedback = stageB[3] - (input * compensation);
            float saturatedFeedback = std::tanh(feedback * r);

            float I = input - saturatedFeedback;

            // --- First 4-Pole Ladder Component ---
            stageA[0] = (I * g + oldStageA[0]) * h0;
            stageA[1] = (stageA[0] * g + oldStageA[1]) * h0;
            stageA[2] = (stageA[1] * g + oldStageA[2]) * h0;
            stageA[3] = (stageA[2] * g + oldStageA[3]) * h0;

            // --- Second 4-Pole Ladder Component (Fed by Stage A output) ---
            stageB[0] = (stageA[3] * g + oldStageB[0]) * h0;
            stageB[1] = (stageB[0] * g + oldStageB[1]) * h0;
            stageB[2] = (stageB[1] * g + oldStageB[2]) * h0;
            stageB[3] = (stageB[2] * g + oldStageB[3]) * h0;

            for (int i = 0; i < 4; i++) {
                oldStageA[i] = stageA[i];
                oldStageB[i] = stageB[i];
            }
        }

        return stageB[3];
    }

    // --- Spatialization & Delay Core Processors ---
    float delayProcess(float sig, float mix, float timeMs, float fdbk)
    {
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
    char lfoTargetName[24] = "Off";
    const char* lfoTargetStrings[3] = { "Off", "Morph", "Pitch" };

    Param params[18];

    // Modern Parameter Registrations
    Param& freq = addParam({ .key = "frequency", .label = "Frequency", .unit = "Hz", .value = 130.81f, .min = 20.0f, .max = 800.0f, .step = 0.1f });
    Param& morph = addParam({ .key = "morph", .label = "Morph", .unit = "%", .value = 0.0f });
    Param& pw = addParam({ .key = "pulseWidth", .label = "Pulse Width", .unit = "%", .value = 50.0f, .min = 5.0f, .max = 95.0f });
    Param& wavefold = addParam({ .key = "wavefold", .label = "Wavefold", .unit = "%", .value = 0.0f });
    Param& subMix = addParam({ .key = "subMix", .label = "Sub Mix", .unit = "%", .value = 0.0f });
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .unit = "%", .value = 60.0f });
    Param& resonance = addParam({ .key = "resonance", .label = "Resonance", .unit = "%", .value = 20.0f });
    Param& envMod = addParam({ .key = "envMod", .label = "Env Mod", .unit = "%", .value = 40.0f });
    Param& decayTime = addParam({ .key = "decay", .label = "Decay", .unit = "ms", .value = 250.0f, .min = 10.0f, .max = 2000.0f, .step = 5.0f });
    Param& accentAmt = addParam({ .key = "accent", .label = "Accent", .unit = "%", .value = 50.0f });
    Param& hpCutoff = addParam({ .key = "hpCut", .label = "HP Cut", .unit = "%", .value = 10.0f });
    Param& lfoRate = addParam({ .key = "lfoRate", .label = "LFO Rate", .unit = "Hz", .value = 1.5f, .min = 0.05f, .max = 30.0f, .step = 0.05f });
    Param& lfoTarget = addParam({ .key = "lfoTarget", .label = "LFO Target", .string = lfoTargetName, .value = 0.0f, .min = 0.0f, .max = 2.0f, .step = 1.0f, // Skip Format
        .onUpdate = [](void* ctx, float v) { // Skip Format
            auto e = (VoidBass*)ctx;
            int idx = std::clamp((int)v, 0, 2);
            strcpy(e->lfoTargetName, e->lfoTargetStrings[idx]);
        }, // Skip Format
        .setStringFn = [](void* ctx, float value, char* str) { // Skip Format
            auto e = (VoidBass*)ctx;
            int idx = std::clamp((int)value, 0, 2);
            strcpy(str, e->lfoTargetStrings[idx]);
        }, // Skip Format
        .stringToFloatFn = [](void* ctx, const char* valStr) { // Skip Format
            auto e = (VoidBass*)ctx;
            for (int i = 0; i < 3; ++i) {
                if (strcmp(valStr, e->lfoTargetStrings[i]) == 0) return (float)i;
            }
            return 0.0f; // Default fallback to "Off"
        } });
    Param& glide = addParam({ .key = "glide", .label = "Glide", .unit = "ms", .value = 0.0f, .max = 1000.0f, .step = 5.0f });
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 10.0f });
    Param& dlyMix = addParam({ .key = "dlyMix", .label = "Dly Mix", .unit = "%", .value = 0.0f });
    Param& dlyTime = addParam({ .key = "dlyTime", .label = "Dly Time", .unit = "ms", .value = 250.0f, .min = 10.0f, .max = 1000.0f, .step = 5.0f });
    Param& dlyFdbk = addParam({ .key = "dlyFdbk", .label = "Dly Fdbk", .unit = "%", .value = 30.0f });

    VoidBass(float sr, float* dlBuf)
        : EngineBase(Synth, "VoidBass", params)
        , sampleRate(sr)
        , sampleRateDiv(1.0f / sr)
        , delayBuf(dlBuf)
    {
        init();
    }

    void noteOnImpl(uint8_t note, float vel)
    {
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

    float sampleImpl()
    {
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
        if (!gateOpen) {
            vcfEnv *= tau(decayTime.value);
            float decayMod = tau(accented ? decayTime.value * 0.5f : decayTime.value);
            accentVcf *= decayMod;
            accentVca *= decayMod;
        }

        // 7. Elite Lowpass 48dB Cascaded Ladder Filtering
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