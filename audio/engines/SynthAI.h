#pragma once

#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"
#include "helpers/clamp.h"
#include <algorithm>
#include <cmath>
#include <cstring>

class SynthAI : public EngineBase<SynthAI> {
public:
    static constexpr int FX_BUF_SIZE = 16384;

protected:
    const float sampleRate;
    const float invSampleRate;

    // Oscillators
    float phase1 = 0.0f;
    float phase2 = 0.0f;
    float noiseZ = 0.0f;

    // Envelopes
    float ampEnv = 0.0f;
    float modEnv = 0.0f;
    float pitchEnv = 1.0f;
    int voiceStage = 0;

    // Filter
    FilterSVF filter;

    // UI String Buffers (To prevent the null string crash)
    char osc1WaveName[16] = "Sine";
    char osc2WaveName[16] = "Sine";
    char modModeName[16] = "Add";

    float getWave(float phase, float type)
    {
        int t = (int)type;
        if (t == 0) return Math::fastSin(PI_X2 * phase); // Sine
        if (t == 1) return (phase < 0.5f) ? 1.0f : -1.0f; // Square
        if (t == 2) return (phase * 2.0f) - 1.0f; // Saw
        if (t == 3) return 1.0f - std::abs((phase * 4.0f) - 2.0f); // Triangle
        return 0.0f;
    }

    float tau(float ms) const
    {
        return (ms < 0.5f) ? 0.0f : Math::exp(-1.0f / (sampleRate * ms * 0.001f));
    }

    static const char* waveToName(int type)
    {
        switch (type) {
        case 1:
            return "Square";
        case 2:
            return "Saw";
        case 3:
            return "Triangle";
        default:
            return "Sine";
        }
    }

public:
    Param params[19] = {
        // --- OSC 1 ---
        { .label = "Osc1 Wave", .string = osc1WaveName, .value = 0.0f, .min = 0.0f, .max = 3.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
             auto* s = (SynthAI*)ctx;
             strcpy(s->osc1WaveName, waveToName((int)val));
         } },
        { .label = "Osc1 Mult", .value = 1.0f, .min = 0.5f, .max = 16.0f, .step = 0.5f },

        // --- OSC 2 ---
        { .label = "Mod Mode", .string = modModeName, .value = 0.0f, .min = 0.0f, .max = 1.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
             auto* s = (SynthAI*)ctx;
             strcpy(s->modModeName, (val > 0.5f) ? "FM" : "Add");
         } },
        { .label = "Osc2 Wave", .string = osc2WaveName, .value = 0.0f, .min = 0.0f, .max = 3.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
             auto* s = (SynthAI*)ctx;
             strcpy(s->osc2WaveName, waveToName((int)val));
         } },
        { .label = "Osc2 Mult", .value = 2.0f, .min = 0.5f, .max = 16.0f, .step = 0.1f }, { .label = "Mod Depth", .unit = "%", .value = 20.0f },

        // --- PITCH ---
        { .label = "P.Env Amt", .unit = "st", .value = 0.0f, .min = 0.0f, .max = 48.0f }, { .label = "P.Env Dec", .unit = "ms", .value = 50.0f, .min = 2.0f, .max = 500.0f },

        // --- NOISE ---
        { .label = "Noise Mix", .unit = "%", .value = 0.0f }, { .label = "Punch", .unit = "%", .value = 0.0f },

        // --- FILTER ---
        { .label = "Cutoff", .unit = "%", .value = 100.0f }, { .label = "Res", .unit = "%", .value = 10.0f }, { .label = "F.Env Amt", .unit = "%", .value = 0.0f },

        // --- ENVELOPES ---
        { .label = "Amp Att", .unit = "ms", .value = 2.0f, .min = 1.0f, .max = 1000.0f }, { .label = "Amp Dec", .unit = "ms", .value = 200.0f, .min = 5.0f, .max = 3000.0f }, { .label = "Mod Att", .unit = "ms", .value = 2.0f, .min = 1.0f, .max = 1000.0f }, { .label = "Mod Dec", .unit = "ms", .value = 100.0f, .min = 5.0f, .max = 3000.0f },

        // --- GLOBAL ---
        { .label = "Master Vol", .unit = "%", .value = 80.0f }, { .label = "Drive", .unit = "%", .value = 0.0f }
    };

    SynthAI(float sr, float* dummy1, float* dummy2)
        : EngineBase(Synth, "SynthAI", params)
        , sampleRate(sr)
        , invSampleRate(1.0f / sr)
    {
        (void)dummy1;
        (void)dummy2;
        init(); // Essential for linking onUpdate pointers
    }

    void noteOnImpl(uint8_t note, float vel)
    {
        voiceStage = 1;
        ampEnv = 0.0f;
        modEnv = 0.0f;
        pitchEnv = 1.0f;
        phase1 = 0.0f;
        phase2 = 0.0f;
        (void)note;
        (void)vel;
    }

    void noteOffImpl(uint8_t) { voiceStage = 3; }

    float sampleImpl()
    {
        if (voiceStage == 0) return 0.0f;

        float aAtt = 1.0f / (sampleRate * params[13].value * 0.001f);
        float aDec = tau(params[14].value);
        float mAtt = 1.0f / (sampleRate * params[15].value * 0.001f);
        float mDec = tau(params[16].value);
        float pDec = tau(params[7].value);

        if (voiceStage == 1) {
            ampEnv += aAtt;
            modEnv += mAtt;
            if (ampEnv >= 1.0f) {
                ampEnv = 1.0f;
                voiceStage = 2;
            }
        } else if (voiceStage == 2 || voiceStage == 3) {
            ampEnv *= aDec;
            modEnv *= mDec;
            if (voiceStage == 3 && ampEnv < 0.001f) voiceStage = 0;
        }
        pitchEnv *= pDec;

        // Using 440.0f as a base since we aren't tracking MIDI note frequency here
        // to keep AI logic simple, but you can map param[0] to note frequency later.
        float baseFreq = 110.0f;
        float pMod = std::pow(2.0f, (pitchEnv * params[6].value) / 12.0f);
        float freq1 = baseFreq * params[1].value * pMod;
        float freq2 = baseFreq * params[4].value * pMod;

        phase2 += freq2 * invSampleRate;
        if (phase2 >= 1.0f) phase2 -= 1.0f;
        float modSig = getWave(phase2, params[3].value) * modEnv;

        float fmIn = (params[2].value > 0.5f) ? modSig * params[5].value * 0.05f : 0.0f;
        phase1 += freq1 * invSampleRate;
        if (phase1 >= 1.0f) phase1 -= 1.0f;

        float carrier = getWave(std::fmod(phase1 + fmIn, 1.0f), params[0].value);

        float out = carrier;
        if (params[2].value < 0.5f) {
            out = (carrier * 0.5f) + (modSig * params[5].value * 0.01f);
        }

        noiseZ = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        out = (out * (1.0f - params[8].value * 0.01f)) + (noiseZ * params[8].value * 0.01f * ampEnv);

        if (params[9].value > 0.0f && pitchEnv > 0.1f) {
            out += noiseZ * (params[9].value * 0.01f) * pitchEnv;
        }

        float cut = (params[10].value * 0.01f) + (ampEnv * params[12].value * 0.01f);
        filter.setCutoff(std::clamp(cut, 0.01f, 0.99f));
        filter.setResonance(params[11].value * 0.01f);
        out = filter.process12(out).lp;

        if (params[18].value > 0.0f) {
            out = std::tanh(out * (1.0f + params[18].value * 0.02f));
        }

        return out * ampEnv * (params[17].value * 0.01f);
    }
};