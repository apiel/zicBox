#pragma once

#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"
#include "helpers/clamp.h"
#include <cmath>
#include <cstring>

class SynthAI : public EngineBase<SynthAI> {
protected:
    const float sampleRate;
    const float invSampleRate;

    float phases[4] = { 0, 0, 0, 0 };
    float lfoPhases[2] = { 0, 0 };
    float noiseState = 0;
    float ampEnv = 0, fltEnv = 0, modEnv = 0, pitEnv = 1.0f;
    int stage = 0;

    FilterSVF filter1, filter2;

    char oscNames[3][16] = { "Sine", "Sine", "Sine" };
    char lfoNames[2][16] = { "Sine", "Sine" };

    float getWave(float ph, int type)
    {
        ph = std::fmod(ph, 1.0f);
        if (ph < 0) ph += 1.0f;
        switch (type) {
        case 1:
            return (ph < 0.5f) ? 1.0f : -1.0f; // Square
        case 2:
            return (ph * 2.0f) - 1.0f; // Saw
        case 3:
            return 1.0f - std::abs((ph * 4.0f) - 2.0f); // Triangle
        case 4:
            return ((float)rand() / RAND_MAX) * 2.0f - 1.0f; // Noise
        default:
            return Math::fastSin(PI_X2 * ph); // Sine
        }
    }

    float tau(float ms) const
    {
        return (ms < 0.5f) ? 0.0f : Math::exp(-1.0f / (sampleRate * ms * 0.001f));
    }

public:
    Param params[45];

    SynthAI(float sr, float* d1, float* d2)
        : EngineBase(Synth, "SynthAI", params)
        , sampleRate(sr)
        , invSampleRate(1.0f / sr)
    {
        // OSCILLATORS
        params[0] = { .label = "O1 Wave", .string = oscNames[0], .max = 4, .step = 1 };
        params[1] = { .label = "O1 Mult", .value = 1.0f, .max = 32.0f };
        params[2] = { .label = "O1 Level", .unit = "%", .value = 100.0f };
        params[3] = { .label = "O1 Detune", .unit = "ct", .value = 0.0f, .min = -50.0f, .max = 50.0f };

        params[4] = { .label = "O2 Wave", .string = oscNames[1], .max = 4, .step = 1 };
        params[5] = { .label = "O2 Mult", .value = 2.0f, .max = 32.0f };
        params[6] = { .label = "O2 Level", .unit = "%", .value = 0.0f };
        params[7] = { .label = "O2 FM > 1", .unit = "%", .value = 0.0f };

        params[8] = { .label = "O3 Wave", .string = oscNames[2], .max = 4, .step = 1 };
        params[9] = { .label = "O3 Mult", .value = 0.5f, .max = 32.0f };
        params[10] = { .label = "O3 Level", .unit = "%", .value = 0.0f };
        params[11] = { .label = "O3 FM > 2", .unit = "%", .value = 0.0f };

        params[12] = { .label = "Sub Level", .unit = "%", .value = 0.0f };
        params[13] = { .label = "Sub Wave", .value = 0, .max = 1, .step = 1 };
        params[14] = { .label = "Noise Mix", .unit = "%", .value = 0.0f };
        params[15] = { .label = "Noise Color", .value = 0.5f };

        // ENVELOPES
        params[16] = { .label = "Amp Att", .unit = "ms", .value = 2.0f, .max = 2000.0f };
        params[17] = { .label = "Amp Dec", .unit = "ms", .value = 200.0f, .max = 5000.0f };
        params[18] = { .label = "Amp Sus", .unit = "%", .value = 80.0f };
        params[19] = { .label = "Amp Rel", .unit = "ms", .value = 200.0f, .max = 5000.0f };

        params[20] = { .label = "Flt Att", .unit = "ms", .value = 10.0f, .max = 2000.0f };
        params[21] = { .label = "Flt Dec", .unit = "ms", .value = 500.0f, .max = 5000.0f };
        params[22] = { .label = "Flt Sus", .unit = "%", .value = 20.0f };
        params[23] = { .label = "Flt Rel", .unit = "ms", .value = 500.0f, .max = 5000.0f };

        params[24] = { .label = "P.Env Amt", .unit = "st", .value = 0.0f, .min = -48.0f, .max = 48.0f };
        params[25] = { .label = "P.Env Dec", .unit = "ms", .value = 50.0f, .max = 1000.0f };
        params[26] = { .label = "Mod Env Att", .unit = "ms", .value = 2.0f, .max = 1000.0f };
        params[27] = { .label = "Mod Env Dec", .unit = "ms", .value = 100.0f, .max = 1000.0f };

        // FILTERS
        params[28] = { .label = "F1 Cut", .unit = "%", .value = 100.0f };
        params[29] = { .label = "F1 Res", .unit = "%", .value = 10.0f };
        params[30] = { .label = "F1 Env >", .unit = "%", .value = 0.0f };
        params[31] = { .label = "F1 LFO >", .unit = "%", .value = 0.0f };
        params[32] = { .label = "F2 Cut", .unit = "%", .value = 100.0f };
        params[33] = { .label = "F2 Res", .unit = "%", .value = 0.0f };
        params[34] = { .label = "F2 Env >", .unit = "%", .value = 0.0f };
        params[35] = { .label = "F2 Type", .value = 0, .max = 1, .step = 1 }; // 0:LP, 1:HP

        // LFOs & MOD
        params[36] = { .label = "LFO1 Rate", .unit = "Hz", .value = 5.0f, .max = 50.0f };
        params[37] = { .label = "LFO1 Wave", .string = lfoNames[0], .max = 4, .step = 1 };
        params[38] = { .label = "LFO1 > Pit", .unit = "st", .value = 0.0f };
        params[39] = { .label = "LFO2 Rate", .unit = "Hz", .value = 0.5f, .max = 50.0f };
        params[40] = { .label = "LFO2 Wave", .string = lfoNames[1], .max = 4, .step = 1 };
        params[41] = { .label = "LFO2 > FM", .unit = "%", .value = 0.0f };

        // GLOBAL / FX
        params[42] = { .label = "Drive", .unit = "%", .value = 0.0f };
        params[43] = { .label = "Bitcrush", .unit = "%", .value = 0.0f };
        params[44] = { .label = "Master", .unit = "%", .value = 80.0f };

        init();
    }

    void noteOnImpl(uint8_t note, float vel)
    {
        stage = 1;
        for (int i = 0; i < 4; i++)
            phases[i] = 0;
        pitEnv = 1.0f;
        fltEnv = 0.0f;
        modEnv = 0.0f;
    }

    void noteOffImpl(uint8_t n) { stage = 4; }

    float sampleImpl()
    {
        if (stage == 0) return 0;

        // 1. Envelope Processing
        float aSus = params[18].value * 0.01f;
        float fSus = params[22].value * 0.01f;

        if (stage == 1) { // Attack
            ampEnv += 1.0f / (sampleRate * params[16].value * 0.001f);
            fltEnv += 1.0f / (sampleRate * params[20].value * 0.001f);
            modEnv += 1.0f / (sampleRate * params[26].value * 0.001f);
            if (ampEnv >= 1.0f) {
                ampEnv = 1.0f;
                stage = 2;
            }
            if (fltEnv >= 1.0f) fltEnv = 1.0f;
            if (modEnv >= 1.0f) modEnv = 1.0f;
        } else if (stage == 2) { // Decay/Sustain
            ampEnv = aSus + (ampEnv - aSus) * tau(params[17].value);
            fltEnv = fSus + (fltEnv - fSus) * tau(params[21].value);
            modEnv *= tau(params[27].value);
        } else if (stage == 4) { // Release
            ampEnv *= tau(params[19].value);
            fltEnv *= tau(params[23].value);
            if (ampEnv < 0.001f) stage = 0;
        }
        pitEnv *= tau(params[25].value);

        // 2. LFOs
        lfoPhases[0] += params[36].value * invSampleRate;
        lfoPhases[1] += params[39].value * invSampleRate;
        float lfo1 = getWave(lfoPhases[0], (int)params[37].value);
        float lfo2 = getWave(lfoPhases[1], (int)params[40].value);

        // 3. FM Engine
        float baseFreq = 110.0f;
        float pMod = std::pow(2.0f, (pitEnv * params[24].value + lfo1 * params[38].value) / 12.0f);

        float fm3to2 = (params[11].value * 0.1f) * modEnv;
        float mod3 = getWave(phases[2], (int)params[8].value) * fm3to2;
        phases[2] += (baseFreq * params[9].value * pMod) * invSampleRate;

        float fm2to1 = (params[7].value * 0.1f) * (1.0f + lfo2 * params[41].value * 0.01f);
        float mod2 = getWave(phases[1] + mod3, (int)params[4].value) * fm2to1;
        phases[1] += (baseFreq * params[5].value * pMod) * invSampleRate;

        float detune = 1.0f + (params[3].value * 0.0001f);
        float carrier = getWave(phases[0] + mod2, (int)params[0].value);
        phases[0] += (baseFreq * params[1].value * pMod * detune) * invSampleRate;

        // 4. Mixing
        float sig = (carrier * params[2].value * 0.01f) + (getWave(phases[1], (int)params[4].value) * params[6].value * 0.01f);
        sig += getWave(phases[3], (int)params[13].value) * params[12].value * 0.01f; // Sub
        phases[3] += (baseFreq * 0.5f * pMod) * invSampleRate;

        float white = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        noiseState += (params[15].value * 0.5f) * (white - noiseState);
        sig += noiseState * params[14].value * 0.01f;

        // 5. Dual Filters
        float cut1 = (params[28].value * 0.01f) + (fltEnv * params[30].value * 0.01f) + (lfo1 * params[31].value * 0.01f);
        filter1.setCutoff(std::clamp(cut1, 0.01f, 0.99f));
        filter1.setResonance(params[29].value * 0.01f);
        sig = filter1.process12(sig).lp;

        float cut2 = (params[32].value * 0.01f) + (fltEnv * params[34].value * 0.01f);
        filter2.setCutoff(std::clamp(cut2, 0.01f, 0.99f));
        filter2.setResonance(params[33].value * 0.01f);
        sig = (params[35].value > 0.5f) ? filter2.process12(sig).hp : filter2.process12(sig).lp;

        // 6. FX & Master
        if (params[42].value > 0) sig = std::tanh(sig * (1.0f + params[42].value * 0.1f));
        if (params[43].value > 0) {
            float step = std::pow(0.5f, params[43].value * 0.1f);
            sig = std::floor(sig / step) * step;
        }

        return sig * ampEnv * (params[44].value * 0.01f);
    }
};