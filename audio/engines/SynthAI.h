#pragma once

#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"
#include <algorithm>
#include <cmath>

class SynthAI : public EngineBase<SynthAI> {
protected:
    float sampleRate;
    float invSampleRate;

    // Oscillator & LFO Phases
    float phase1 = 0.0f;
    float phase2 = 0.0f;
    float lfoPhase[2] = { 0.0f, 0 };
    float lfoSHValue[2] = { 0.0f, 0 };

    struct ADSR {
        float value = 0.0f;
        int state = 0; // 0:Idle, 1:Attack, 2:Decay, 3:Sustain, 4:Release
    };
    ADSR env1, env2;

    float currentNote = 60.0f;
    float targetNote = 60.0f;
    bool gateOpen = false;
    uint32_t seed = 12345;

    FilterSVF filter;

    // --- Internal Helpers ---
    float fastNoise()
    {
        seed = 214013 * seed + 2531011;
        return ((float)((seed >> 16) & 0x7FFF) / 32768.0f) * 2.0f - 1.0f;
    }

    float oscWave(float ph, int type)
    {
        ph -= std::floor(ph);
        switch (type) {
        case 1:
            return (2.0f * ph - 1.0f); // Saw
        case 2:
            return (ph < 0.5f) ? 1.0f : -1.0f; // Square
        case 3:
            return (ph < 0.5f) ? (4.0f * ph - 1.0f) : (3.0f - 4.0f * ph); // Triangle
        default:
            return std::sin(6.2831853f * ph); // Sine
        }
    }

    float processADSR(ADSR& env, float a, float d, float s, float r)
    {
        float attackStep = a < 1.0f ? 1.0f : 1.0f / (sampleRate * a * 0.001f);
        float decayCoeff = std::exp(-1.0f / (sampleRate * std::max(1.0f, d) * 0.001f));
        float releaseCoeff = std::exp(-1.0f / (sampleRate * std::max(1.0f, r) * 0.001f));

        switch (env.state) {
        case 1:
            env.value += attackStep;
            if (env.value >= 1.0f) {
                env.value = 1.0f;
                env.state = 2;
            }
            break;
        case 2:
            env.value = s + (env.value - s) * decayCoeff;
            break;
        case 4:
            env.value *= releaseCoeff;
            if (env.value < 0.001f) {
                env.value = 0.0f;
                env.state = 0;
            }
            break;
        }
        return env.value;
    }

    float* reverbBuf = nullptr;

    static constexpr int COMB_LEN[4] = { 1559, 1617, 1685, 1751 };
    static constexpr int AP_LEN[3] = { 347, 113, 37 };

    int combOff[4] = {};
    int apOff[3] = {};
    int combIdx[8] = {};
    int apIdx[4] = {};
    float combFb[8] = {};
    float reverbProcess(float in, float mix, float damp)
    {
        if (mix < 0.001f) return in;
        const float decay = 0.88f;
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

    static constexpr int DELAY_BUF_SIZE = 44100;
    float* delayBuf = nullptr;
    int delayWrite = 0;
    float dlyFbSmooth = 0.0f;
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

        return sig + (sig + delayed * 0.7f) * (dlyMix.value * 0.01f - sig);
    }

    float bufferedFxProcess(float sig)
    {
        sig = delayProcess(sig);
        sig = reverbProcess(sig, reverbMix.value * 0.01f, reverbDamp.value * 0.01f);
        return sig;
    }

public:
    Param params[45] = {
        { .label = "Osc1 Wave", .value = 0.0f, .max = 3.0f }, // Sine, Saw, Sqr, Tri
        { .label = "Osc1 Freq", .unit = "st", .value = 0.0f, .min = -24, .max = 24.0f },
        { .label = "Osc1 Env1 Freq", .value = 0.0f, .min = -48.0f, .max = 48.0f },
        { .label = "Osc1 Env2 Freq", .value = 0.0f, .min = -48.0f, .max = 48.0f },
        { .label = "Osc1 LFO1 Freq", .value = 0.0f, .min = -24.0f, .max = 24.0f },
        { .label = "Osc1 LFO2 Freq", .value = 0.0f, .min = -24.0f, .max = 24.0f },
        { .label = "Osc1 Env1 Amp", .value = 100.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Osc1 Env2 Amp", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Osc1 LFO1 Amp", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Osc1 LFO2 Amp", .value = 0.0f, .min = -100.0f, .max = 100.0f },

        { .label = "Osc2 Wave", .value = 0.0f, .max = 3.0f },
        { .label = "Osc2 Ratio", .value = 1.0f, .min = 0.5f, .max = 16.0f },
        { .label = "Osc2 Env2 Ratio", .value = 0.0f, .min = -8.0f, .max = 8.0f },
        { .label = "Osc2 LFO2 Ratio", .value = 0.0f, .min = -8.0f, .max = 8.0f },
        { .label = "Osc2 Env1 Amp", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Osc2 Env2 Amp", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Osc2 LFO2 Amp", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Osc2 > Osc1 FM", .value = 0.0f, .max = 100.0f },

        { .label = "Cutoff", .value = 80.0f, .max = 100.0f },
        { .label = "Resonance", .value = 10.0f, .max = 100.0f },
        { .label = "Filter Type", .value = 0.0f, .max = 2.0f },
        { .label = "Env1 Cutoff", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Env2 Cutoff", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "LFO1 Cutoff", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "LFO2 Cutoff", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "LFO2 Resonance", .value = 0.0f, .min = -100.0f, .max = 100.0f },

        { .label = "LFO1 Rate", .unit = "Hz", .value = 2.0f, .max = 400.0f },
        { .label = "LFO2 Rate", .unit = "Hz", .value = 0.5f, .max = 400.0f },

        { .label = "Env1 Attack", .value = 5.0f },
        { .label = "Env1 Decay", .value = 200.0f },
        { .label = "Env1 Sustain", .value = 50.0f },
        { .label = "Env1 Release", .value = 300.0f },

        { .label = "Env2 Attack", .value = 10.0f },
        { .label = "Env2 Decay", .value = 400.0f },
        { .label = "Env2 Sustain", .value = 0.0f },
        { .label = "Env2 Release", .value = 500.0f },

        { .label = "Drive", .value = 0.0f, .max = 100.0f },
        { .label = "Waveshaper", .value = 0.0f, .max = 100.0f },
        { .label = "Decimator", .value = 0.0f, .max = 100.0f },
        { .label = "Glide", .value = 0.0f, .max = 500.0f },

        { .label = "Reverb Mix", .unit = "%", .value = 0.0f },
        { .label = "Rvb Damp", .unit = "%", .value = 50.0f },
        { .label = "Dly Mix", .unit = "%", .value = 0.0f },
        { .label = "Dly Time", .unit = "ms", .value = 125.0f, .min = 10.0f, .max = 1000.0f, .step = 5.0f },
        { .label = "Dly Fdbk", .unit = "%", .value = 0.0f },
    };

    Param& reverbMix = params[40];
    Param& reverbDamp = params[41];
    Param& dlyMix = params[42];
    Param& dlyTime = params[43];
    Param& dlyFdbk = params[44];

    SynthAI(float sr, float* dlBuf, float* rvBuf)
        : EngineBase(Synth, "TeknoSynth", params)
        , delayBuf(dlBuf)
        , reverbBuf(rvBuf)
    {
        sampleRate = sr;
        invSampleRate = 1.0f / sr;
    }

    void noteOnImpl(uint8_t note, float vel)
    {
        targetNote = (float)note;
        if (!gateOpen) currentNote = targetNote;
        gateOpen = true;
        env1.state = env2.state = 1;
    }

    void noteOffImpl(uint8_t note)
    {
        gateOpen = false;
        env1.state = env2.state = 4;
    }

    float sampleImpl()
    {
        // 1. Process Envelopes
        float e1 = processADSR(env1, params[28].value, params[29].value, params[30].value * 0.01f, params[31].value);
        float e2 = processADSR(env2, params[32].value, params[33].value, params[34].value * 0.01f, params[35].value);

        if (env1.state == 0 && env2.state == 0) return bufferedFxProcess(0.0f);

        // 2. Process LFOs
        for (int i = 0; i < 2; i++) {
            lfoPhase[i] += params[26 + i].value * invSampleRate;
            if (lfoPhase[i] >= 1.0f) lfoPhase[i] -= 1.0f;
        }
        float lfo1 = std::sin(6.2831853f * lfoPhase[0]);
        float lfo2 = std::sin(6.2831853f * lfoPhase[1]);

        // 3. Pitch & Phase
        if (params[38].value > 0) {
            float glideCoeff = std::exp(-1.0f / (sampleRate * params[38].value * 0.001f));
            currentNote = targetNote + glideCoeff * (currentNote - targetNote);
        } else currentNote = targetNote;

        float basePitch = currentNote + params[1].value;
        basePitch += (e1 * params[2].value) + (e2 * params[3].value);
        basePitch += (lfo1 * params[4].value) + (lfo2 * params[5].value);
        float freq1 = 440.0f * std::pow(2.0f, (basePitch - 69.0f) / 12.0f);

        // Osc 2 Ratio Modulation
        float ratio2 = params[11].value + (e2 * params[12].value) + (lfo2 * params[13].value);
        float freq2 = freq1 * std::max(0.1f, ratio2);

        // 4. Oscillators (FM Logic)
        float osc2Amp = std::clamp((e1 * params[14].value + e2 * params[15].value + lfo2 * params[16].value) * 0.01f, 0.0f, 1.0f);
        float s2 = oscWave(phase2, (int)params[10].value) * osc2Amp;
        phase2 += freq2 * invSampleRate;

        // FM Modulation into Osc1 phase
        float fmMod = s2 * params[17].value * 0.05f;
        float osc1Amp = std::clamp((e1 * params[6].value + e2 * params[7].value + lfo1 * params[8].value + lfo2 * params[9].value) * 0.01f, 0.0f, 1.0f);
        float s1 = oscWave(phase1 + fmMod, (int)params[0].value) * osc1Amp;
        phase1 += freq1 * invSampleRate;

        float sig = s1 + s2;

        // 5. Filter
        float cutoffMod = params[18].value * 0.01f + (e1 * params[21].value * 0.01f) + (e2 * params[22].value * 0.01f) + (lfo1 * params[23].value * 0.01f) + (lfo2 * params[24].value * 0.01f);
        float resMod = std::clamp(params[19].value * 0.01f + (lfo2 * params[25].value * 0.01f), 0.0f, 0.99f);

        filter.setCutoff(std::clamp(cutoffMod * cutoffMod, 0.01f, 0.99f));
        filter.setResonance(resMod);
        auto fOut = filter.process12(sig);
        int fType = (int)params[20].value;
        sig = (fType == 0) ? fOut.lp : (fType == 1 ? fOut.hp : fOut.bp);

        // 6. Distortion & Mangle (Tekno Style)
        // Waveshaper: Recursive Sine Folder
        if (params[37].value > 0.1f) {
            float foldAmount = params[37].value * 0.2f;
            sig = std::sin(sig * (1.0f + foldAmount * 5.0f)); // First fold
            sig = std::sin(sig * (1.0f + foldAmount * 2.0f)); // Second fold for grit
        }

        // Drive: Soft Saturator
        if (params[36].value > 0.1f) {
            float drive = 1.0f + (params[36].value * 0.1f);
            sig = std::tanh(sig * drive);
        }

        // Decimator
        static float lastSig = 0;
        static float deciCounter = 0;
        deciCounter += (1.0f - (params[38].value * 0.0099f));
        if (deciCounter >= 1.0f) {
            deciCounter -= 1.0f;
            lastSig = sig;
        }
        sig = lastSig;

        return bufferedFxProcess(sig * 0.5f);
    }
};