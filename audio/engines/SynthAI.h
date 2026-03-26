#pragma once

#include "audio/effects/applyWaveshape.h"
#include "audio/engines/EngineBase.h"
#include "audio/filterMoog.h"
#include "audio/filterSVF.h"
#include "audio/filterTB.h"
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

    struct ADSR {
        float value = 0.0f;
        int state = 0; // 0:Idle, 1:Attack, 2:Decay, 3:Sustain, 4:Release
    };
    ADSR env1, env2;

    float currentNote = 60.0f;
    float targetNote = 60.0f;
    bool gateOpen = false;
    uint32_t seed = 12345;

    char osc1WaveName[16] = "Saw";
    char osc2WaveName[16] = "Sine";

    char lfo1TypeName[16] = "Sin";
    char lfo2TypeName[16] = "Sin";

    static constexpr const char* WAVE_NAMES[7] = { "Sine", "Saw", "Square", "Triangle", "White", "Pink", "Brown" };
    static constexpr const char* LFO_NAMES[13] = { "Sin", "Saw", "Tri", "Sqr", "Sin Trg", "Saw Trg", "Tri Trg", "Sqr Trg", "Sin One", "Saw One", "Tri One", "Sqr One", "S&H" };

    float fastNoise()
    {
        seed = 214013 * seed + 2531011;
        return ((float)((seed >> 16) & 0x7FFF) / 32768.0f) * 2.0f - 1.0f;
    }

    float pinkStore[7] = { 0 };
    float brownStore = 0.0f;
    float getPinkNoise()
    {
        float white = fastNoise();
        pinkStore[0] = 0.99886f * pinkStore[0] + white * 0.0555179f;
        pinkStore[1] = 0.99332f * pinkStore[1] + white * 0.0750377f;
        pinkStore[2] = 0.96900f * pinkStore[2] + white * 0.1538520f;
        pinkStore[3] = 0.86650f * pinkStore[3] + white * 0.3104856f;
        pinkStore[4] = 0.55000f * pinkStore[4] + white * 0.5329522f;
        pinkStore[5] = -0.7616f * pinkStore[5] - white * 0.0168980f;
        float pink = pinkStore[0] + pinkStore[1] + pinkStore[2] + pinkStore[3] + pinkStore[4] + pinkStore[5] + pinkStore[6] + white * 0.5362f;
        pinkStore[6] = white * 0.115926f;
        return pink * 0.11f;
    }

    float getBrownNoise()
    {
        float white = fastNoise();
        brownStore = (brownStore + (0.02f * white)) / 1.02f;
        return brownStore * 3.5f;
    }

    float lfoPhase[2] = { 0.0f, 0 };
    float lfoSHValue[2] = { 0.0f, 0 };
    bool lfoDone[2] = { false, false };

    float oscWave(float ph_in, int type)
    {
        ph_in -= std::floor(ph_in);
        switch (type) {
        case 1:
            return (2.0f * ph_in - 1.0f);
        case 2:
            return (ph_in < 0.5f) ? 1.0f : -1.0f;
        case 3:
            return (ph_in < 0.5f) ? (4.0f * ph_in - 1.0f) : (3.0f - 4.0f * ph_in);
        case 4:
            return fastNoise();
        case 5:
            return getPinkNoise();
        case 6: {
            bool lfoDone[2] = { false, false };
            return getBrownNoise();
        }
        default:
            return Math::fastSin(6.2831853f * ph_in);
        }
    }

    float lfoWaveSelect(int index, float ph_in, int type)
    {
        ph_in -= std::floor(ph_in);
        int shape = type % 4;
        if (type == 12) return lfoSHValue[index];
        switch (shape) {
        case 1:
            return (1.0f - 2.0f * ph_in);
        case 2:
            return (ph_in < 0.5f) ? (4.0f * ph_in - 1.0f) : (3.0f - 4.0f * ph_in);
        case 3:
            return (ph_in < 0.5f) ? 1.0f : -1.0f;
        default:
            return Math::fastSin(6.2831853f * ph_in);
        }
    }

    float lfoProcess(int index, float rate, int type)
    {
        if (!lfoDone[index]) {
            lfoPhase[index] += rate * invSampleRate;
            if (lfoPhase[index] >= 1.0f) {
                if (type >= 8 && type <= 11) {
                    lfoPhase[index] = 1.0f;
                    lfoDone[index] = true;
                } else lfoPhase[index] -= 1.0f;
                if (type == 12) lfoSHValue[index] = fastNoise();
            }
        }
        return lfoWaveSelect(index, lfoPhase[index], type);
    }

    float processADSR(ADSR& env, float a, float d, float s, float r)
    {
        float attackStep = a < 1.0f ? 1.0f : 1.0f / (sampleRate * a * 0.0001f);
        float decayCoeff = std::exp(-1.0f / (sampleRate * std::max(1.0f, d) * 0.0001f));
        float releaseCoeff = std::exp(-1.0f / (sampleRate * std::max(1.0f, r) * 0.0001f));

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

    char filterType[32] = "Array 12";
    FilterSVF svfFilter;
    FilterTB tbFilter;
    FilterMoog moogFilter;

    typedef float (SynthAI::*FilterPtr)(float, float, float);
    FilterPtr applyFilterFn = nullptr;

    float applySvf12(float in, float c, float r)
    {
        svfFilter.setCutoff(c);
        svfFilter.setResonance(r);
        return svfFilter.process12(in).lp;
    }
    float applySvf24(float in, float c, float r)
    {
        svfFilter.setCutoff(c);
        svfFilter.setResonance(r);
        return svfFilter.processLp24(in);
    }
    float applyArray12(float in, float c, float r)
    {
        svfFilter.setCutoff(c);
        svfFilter.setResonance(r);
        return svfFilter.processArray12(in).lp;
    }
    float applyArray24(float in, float c, float r)
    {
        svfFilter.setCutoff(c);
        svfFilter.setResonance(r);
        return svfFilter.processArrayLp24(in);
    }
    float applyTbFilter(float in, float c, float r)
    {
        tbFilter.set(c, r);
        return tbFilter.getSample(in);
    }
    float applyMoogFilter(float in, float c, float r)
    {
        moogFilter.setCutoff(c);
        moogFilter.setResonance(r);
        return moogFilter.process(in);
    }
    float applySvfHp(float in, float c, float r)
    {
        svfFilter.setCutoff(c);
        svfFilter.setResonance(r);
        return svfFilter.process12(in).hp;
    }
    float applySvfBp(float in, float c, float r)
    {
        svfFilter.setCutoff(c);
        svfFilter.setResonance(r);
        return svfFilter.process12(in).bp;
    }

public:
    Param params[47] = {
        { .label = "Osc1 Wave", .string = osc1WaveName, .value = 1.0f, .max = 6.0f, .onUpdate = [](void* c, float v) { strncpy(((SynthAI*)c)->osc1WaveName, WAVE_NAMES[(int)v], 15); } },
        { .label = "Osc1 Freq", .unit = "hz", .value = 440.0f, .min = 10, .max = 2000.0f },
        { .label = "Osc1 Env1 Freq", .value = 0.0f, .min = -48.0f, .max = 48.0f },
        { .label = "Osc1 Env2 Freq", .value = 0.0f, .min = -48.0f, .max = 48.0f },
        { .label = "Osc1 LFO1 Freq", .value = 0.0f, .min = -24.0f, .max = 24.0f },
        { .label = "Osc1 LFO2 Freq", .value = 0.0f, .min = -24.0f, .max = 24.0f },
        { .label = "Osc1 Env1 Amp", .value = 100.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Osc1 Env2 Amp", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Osc1 LFO1 Amp", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Osc1 LFO2 Amp", .value = 0.0f, .min = -100.0f, .max = 100.0f },

        { .label = "Osc2 Wave", .string = osc2WaveName, .value = 0.0f, .max = 6.0f, .onUpdate = [](void* c, float v) { strncpy(((SynthAI*)c)->osc2WaveName, WAVE_NAMES[(int)v], 15); } },
        { .label = "Osc2 Ratio", .value = 1.0f, .min = 0.5f, .max = 16.0f },
        { .label = "Osc2 Env2 Ratio", .value = 0.0f, .min = -8.0f, .max = 8.0f },
        { .label = "Osc2 LFO2 Ratio", .value = 0.0f, .min = -8.0f, .max = 8.0f },
        { .label = "Osc2 Env1 Amp", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Osc2 Env2 Amp", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Osc2 LFO2 Amp", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Osc2 > Osc1 FM", .value = 0.0f, .max = 100.0f },

        { .label = "Cutoff", .value = 80.0f, .max = 100.0f },
        { .label = "Resonance", .value = 10.0f, .max = 100.0f },
        { .label = "Filter Type", .string = filterType, .value = 1.0f, .min = 1.0f, .max = 11.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
             auto* s = (SynthAI*)ctx;
             switch ((int)val) {
             case 2:
                 s->applyFilterFn = &SynthAI::applyArray24;
                 strcpy(s->filterType, "Array 24");
                 break;
             case 3:
                 s->applyFilterFn = &SynthAI::applySvf12;
                 strcpy(s->filterType, "SVF 12");
                 break;
             case 4:
                 s->applyFilterFn = &SynthAI::applySvf24;
                 strcpy(s->filterType, "SVF 24");
                 break;
             case 5:
                 s->applyFilterFn = &SynthAI::applyTbFilter;
                 s->tbFilter.setMode(FilterTB::LP_6);
                 strcpy(s->filterType, "TB 6");
                 break;
             case 6:
                 s->applyFilterFn = &SynthAI::applyTbFilter;
                 s->tbFilter.setMode(FilterTB::LP_12);
                 strcpy(s->filterType, "TB 12");
                 break;
             case 7:
                 s->applyFilterFn = &SynthAI::applyTbFilter;
                 s->tbFilter.setMode(FilterTB::LP_18);
                 strcpy(s->filterType, "TB 18");
                 break;
             case 8:
                 s->applyFilterFn = &SynthAI::applyTbFilter;
                 s->tbFilter.setMode(FilterTB::LP_24);
                 strcpy(s->filterType, "TB 24");
                 break;
             case 9:
                 s->applyFilterFn = &SynthAI::applySvfHp;
                 strcpy(s->filterType, "HP SVF");
                 break;
             case 10:
                 s->applyFilterFn = &SynthAI::applySvfBp;
                 strcpy(s->filterType, "BP SVF");
                 break;
             case 11:
                 s->applyFilterFn = &SynthAI::applyMoogFilter;
                 strcpy(s->filterType, "Moog");
                 break;
             default:
                 s->applyFilterFn = &SynthAI::applyArray12;
                 strcpy(s->filterType, "Array 12");
                 break;
             }
         } },
        { .label = "Env1 Cutoff", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Env2 Cutoff", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "LFO1 Cutoff", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "LFO2 Cutoff", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "LFO2 Resonance", .value = 0.0f, .min = -100.0f, .max = 100.0f },

        { .label = "LFO1 Type", .string = lfo1TypeName, .value = 0.0f, .max = 12.0f, .onUpdate = [](void* c, float v) { strncpy(((SynthAI*)c)->lfo1TypeName, LFO_NAMES[(int)v], 15); } },
        { .label = "LFO1 Rate", .unit = "Hz", .value = 2.0f, .max = 400.0f },
        { .label = "LFO2 Type", .string = lfo2TypeName, .value = 0.0f, .max = 12.0f, .onUpdate = [](void* c, float v) { strncpy(((SynthAI*)c)->lfo2TypeName, LFO_NAMES[(int)v], 15); } },
        { .label = "LFO2 Rate", .unit = "Hz", .value = 0.5f, .max = 400.0f },

        { .label = "Env1 Attack", .unit = "ms", .value = 10.0f, .max = 1000.0f, .step = 10.0f },
        { .label = "Env1 Decay", .unit = "ms", .value = 100.0f, .max = 1000.0f, .step = 10.0f },
        { .label = "Env1 Sustain", .value = 50.0f },
        { .label = "Env1 Release", .unit = "ms", .value = 200.0f, .max = 5000.0f, .step = 10.0f },

        { .label = "Env2 Attack", .unit = "ms", .value = 10.0f, .max = 1000.0f, .step = 10.0f },
        { .label = "Env2 Decay", .unit = "ms", .value = 400.0f, .max = 1000.0f, .step = 10.0f },
        { .label = "Env2 Sustain", .value = 0.0f },
        { .label = "Env2 Release", .unit = "ms", .value = 500.0f, .max = 5000.0f, .step = 10.0f },

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

    Param& lfo1Type = params[26];
    Param& lfo1Rate = params[27];
    Param& lfo2Type = params[28];
    Param& lfo2Rate = params[29];

    Param& env1Attack = params[30];
    Param& env1Decay = params[31];
    Param& env1Sustain = params[32];
    Param& env1Release = params[33];

    Param& env2Attack = params[34];
    Param& env2Decay = params[35];
    Param& env2Sustain = params[36];
    Param& env2Release = params[37];

    Param& drive = params[38];
    Param& waveshaper = params[39];
    Param& decimator = params[40];
    Param& glide = params[41];

    Param& reverbMix = params[42];
    Param& reverbDamp = params[43];
    Param& dlyMix = params[44];
    Param& dlyTime = params[45];
    Param& dlyFdbk = params[46];

    SynthAI(float sr, float* dlBuf, float* rvBuf)
        : EngineBase(Synth, "SynthAI", params)
        , delayBuf(dlBuf)
        , reverbBuf(rvBuf)
        , tbFilter(sr)
    {
        sampleRate = sr;
        invSampleRate = 1.0f / sr;

        applyFilterFn = &SynthAI::applyArray12;

        init();
    }

    void noteOnImpl(uint8_t note, float vel)
    {
        targetNote = (float)note;
        if (!gateOpen) currentNote = targetNote;
        gateOpen = true;
        env1.state = env2.state = 1;

        for (int i = 0; i < 2; ++i) {
            int type = (int)(i == 0 ? lfo1Type.value : lfo2Type.value);
            if (type >= 4) {
                lfoPhase[i] = 0.0f;
                lfoDone[i] = false;
            }
            if (type == 12) lfoSHValue[i] = fastNoise();
        }
    }

    void noteOffImpl(uint8_t note)
    {
        gateOpen = false;
        env1.state = env2.state = 4;
    }

    float sampleImpl()
    {
        // 1. Process Envelopes
        float e1 = processADSR(env1, env1Attack.value, env1Decay.value, env1Sustain.value, env1Release.value);
        float e2 = processADSR(env2, env2Attack.value, env2Decay.value, env2Sustain.value, env2Release.value);

        if (env1.state == 0 && env2.state == 0) return bufferedFxProcess(0.0f);

        // 2. Process LFOs
        float lfo1 = lfoProcess(0, lfo1Rate.value * 0.01f, lfo1Type.value);
        float lfo2 = lfoProcess(1, lfo2Rate.value * 0.01f, lfo2Type.value);

        // 3. Pitch & Phase
        if (glide.value > 0) {
            float glideCoeff = std::exp(-1.0f / (sampleRate * params[41].value * 0.001f));
            currentNote = targetNote + glideCoeff * (currentNote - targetNote);
        } else currentNote = targetNote;

        // Convert Osc1 Freq (Hz) to semitones relative to A4 (69)
        float osc1FreqSt = 12.0f * std::log2(params[1].value / 440.0f) + 69.0f;

        // Use the relative offset from the incoming MIDI note
        float basePitch = currentNote + (osc1FreqSt - 60.0f);
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

        sig = std::clamp(sig, -1.0f, 1.0f);
        sig = (this->*applyFilterFn)(sig, std::clamp(cutoffMod * cutoffMod, 0.01f, 0.99f), resMod);

        // 6. Distortion & Mangle

        // Waveshaper
        if (waveshaper.value > 0.1f) {
            sig = applyWaveshape2(sig, waveshaper.value * 0.01f);
        }

        // Drive: Soft Saturator
        if (drive.value > 0.1f) {
            float d = 1.0f + (drive.value * 0.1f);
            sig = std::tanh(sig * d);
        }

        // Decimator
        static float lastSig = 0;
        static float deciCounter = 0;
        deciCounter += (1.0f - (decimator.value * 0.0099f));
        if (deciCounter >= 1.0f) {
            deciCounter -= 1.0f;
            lastSig = sig;
        }
        sig = lastSig;

        return bufferedFxProcess(sig * 0.5f);
    }
};