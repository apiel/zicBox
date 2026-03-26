#pragma once

#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"
#include "helpers/clamp.h"
#include <algorithm>
#include <cmath>
#include <cstring>

class SynthAI : public EngineBase<SynthAI> {
protected:
    float sampleRate;
    float inv;

    float ph[3] = { 0, 0, 0 };
    float phSub = 0.0f;
    float lfoPhase[2] = { 0, 0 };
    bool lfoDone[2] = { false, false };
    float lfoSHValue[2] = { 0, 0 };

    // Pitch Envelope State
    float pitchEnvVal = 0.0f;

    // Noise State
    float pinkStore[7] = { 0 };
    float brownStore = 0.0f;

    struct ADSR {
        float v = 0.0f;
        int st = 0; // 0:Idle, 1:Attack, 2:Decay, 3:Sustain, 4:Release
    };
    ADSR ampEnvelope, filterEnvelope;
    ADSR oscEnvelopes[3];

    float currentNote = 60.0f;
    float targetNote = 60.0f;
    float velocity = 1.0f;
    bool gateOpen = false;

    FilterSVF filter1;
    uint32_t seed = 12345;

    char osc1WaveName[16] = "Saw";
    char osc2WaveName[16] = "Sine";
    char osc3WaveName[16] = "Sine";
    char filter1TypeName[16] = "LP12";

    char lfo1TypeName[16] = "Sin";
    char lfo2TypeName[16] = "Sin";

    static constexpr const char* WAVE_NAMES[7] = { "Sine", "Saw", "Square", "Triangle", "White", "Pink", "Brown" };
    static constexpr const char* FILTER_NAMES[3] = { "LP12", "HP12", "BP12" };
    static constexpr const char* LFO_NAMES[13] = { "Sin", "Saw", "Tri", "Sqr", "Sin Trg", "Saw Trg", "Tri Trg", "Sqr Trg", "Sin One", "Saw One", "Tri One", "Sqr One", "S&H" };

    float fastNoise()
    {
        seed = 214013 * seed + 2531011;
        return ((float)((seed >> 16) & 0x7FFF) / 32768.0f) * 2.0f - 1.0f;
    }

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

    float waveSelect(float ph_in, int type)
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
        case 6:
            return getBrownNoise();
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

    float adsrTick(ADSR& envelope, float attackMs, float decayMs, float sustainLevel, float releaseMs)
    {
        float attackStep = attackMs < 1.0f ? 1.0f : 1.0f / (sampleRate * attackMs * 0.001f);
        float decayCoeff = Math::exp(-1.0f / (sampleRate * std::max(1.0f, decayMs) * 0.001f));
        float releaseCoeff = releaseMs < 1.0f ? 0.0f : Math::exp(-1.0f / (sampleRate * releaseMs * 0.001f));

        switch (envelope.st) {
        case 1:
            envelope.v += attackStep;
            if (envelope.v >= 1.0f) {
                envelope.v = 1.0f;
                envelope.st = 2;
            }
            break;
        case 2:
            envelope.v = sustainLevel + (envelope.v - sustainLevel) * decayCoeff;
            if (std::abs(envelope.v - sustainLevel) < 0.001f) {
                envelope.v = sustainLevel;
                envelope.st = 3;
            }
            break;
        case 3:
            envelope.v = sustainLevel;
            break;
        case 4:
            envelope.v *= releaseCoeff;
            if (envelope.v < 0.0001f) {
                envelope.v = 0.0f;
                envelope.st = 0;
            }
            break;
        }
        return envelope.v;
    }

public:
    Param params[55] = {
        { .label = "Osc1 Wave", .string = osc1WaveName, .value = 1.0f, .max = 6.0f, .onUpdate = [](void* c, float v) { strncpy(((SynthAI*)c)->osc1WaveName, WAVE_NAMES[(int)v], 15); } },
        { .label = "Osc1 Coarse", .unit = "st", .value = 0.0f, .min = -24.0f, .max = 24.0f },
        { .label = "Osc1 Fine", .unit = "ct", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Osc1 Level", .unit = "%", .value = 100.0f },
        { .label = "Osc2 Wave", .string = osc2WaveName, .value = 0.0f, .max = 6.0f, .onUpdate = [](void* c, float v) { strncpy(((SynthAI*)c)->osc2WaveName, WAVE_NAMES[(int)v], 15); } },
        { .label = "Osc2 Coarse", .unit = "st", .value = 0.0f, .min = -24.0f, .max = 24.0f },
        { .label = "Osc2 Level", .unit = "%", .value = 0.0f },
        { .label = "Osc2 Multiplier", .unit = "x", .value = 1.0f, .min = 0.25f, .max = 16.0f },
        { .label = "Osc2 FM Amount", .unit = "%", .value = 0.0f },
        { .label = "Osc3 Wave", .string = osc3WaveName, .value = 0.0f, .max = 6.0f, .onUpdate = [](void* c, float v) { strncpy(((SynthAI*)c)->osc3WaveName, WAVE_NAMES[(int)v], 15); } },
        { .label = "Osc3 Coarse", .unit = "st", .value = -12.0f, .min = -24.0f, .max = 24.0f },
        { .label = "Osc3 Level", .unit = "%", .value = 0.0f },
        { .label = "Osc3 Multiplier", .unit = "x", .value = 1.0f, .min = 0.25f, .max = 16.0f },
        { .label = "Osc3 FM Amount", .unit = "%", .value = 0.0f },
        { .label = "Sub Octave", .value = 1.0f, .max = 2.0f },
        { .label = "Sub Level", .unit = "%", .value = 0.0f },
        { .label = "Sub Waveform", .value = 0.0f, .max = 1.0f },
        { .label = "Amp Attack", .unit = "ms", .value = 5.0f, .max = 3000.0f },
        { .label = "Amp Decay", .unit = "ms", .value = 200.0f, .max = 3000.0f },
        { .label = "Amp Sustain", .unit = "%", .value = 70.0f },
        { .label = "Amp Release", .unit = "ms", .value = 300.0f, .max = 5000.0f },
        { .label = "Amp Velocity", .unit = "%", .value = 50.0f },
        { .label = "Filter Cutoff", .unit = "%", .value = 80.0f },
        { .label = "Filter Resonance", .unit = "%", .value = 10.0f },
        { .label = "Filter Type", .string = filter1TypeName, .value = 0.0f, .max = 2.0f, .onUpdate = [](void* c, float v) { strncpy(((SynthAI*)c)->filter1TypeName, FILTER_NAMES[(int)v], 15); } },
        { .label = "Filter Env Amount", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Filter Sustain", .unit = "%", .value = 0.0f },
        { .label = "Filter Release", .unit = "ms", .value = 300.0f, .max = 5000.0f },
        { .label = "LFO1 Type", .string = lfo1TypeName, .value = 0.0f, .max = 12.0f, .onUpdate = [](void* c, float v) { strncpy(((SynthAI*)c)->lfo1TypeName, LFO_NAMES[(int)v], 15); } },
        { .label = "LFO1 Rate", .unit = "Hz", .value = 5.0f, .max = 50.0f },
        { .label = "LFO1 To Pitch", .unit = "st", .value = 0.0f, .max = 12.0f },
        { .label = "LFO1 To Filter", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "LFO1 To Amp", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "LFO2 Type", .string = lfo2TypeName, .value = 0.0f, .max = 12.0f, .onUpdate = [](void* c, float v) { strncpy(((SynthAI*)c)->lfo2TypeName, LFO_NAMES[(int)v], 15); } },
        { .label = "LFO2 Rate", .unit = "Hz", .value = 0.5f, .max = 50.0f },
        { .label = "LFO2 To Pitch", .unit = "st", .value = 0.0f, .max = 12.0f },
        { .label = "LFO2 To Filter", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "LFO2 To Amp", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Drive", .unit = "%", .value = 0.0f },
        { .label = "Drive Type", .value = 0.0f, .max = 1.0f },
        { .label = "Bitcrush", .unit = "bits", .value = 0.0f, .max = 12.0f },
        { .label = "Decimator", .unit = "%", .value = 0.0f },
        { .label = "Glide", .unit = "ms", .value = 0.0f, .max = 1000.0f },
        { .label = "Osc1 Decay", .unit = "ms", .value = 200.0f, .max = 3000.0f },
        { .label = "Osc1 Sustain", .unit = "%", .value = 100.0f },
        { .label = "Osc2 Decay", .unit = "ms", .value = 200.0f, .max = 3000.0f },
        { .label = "Osc2 Sustain", .unit = "%", .value = 100.0f },
        { .label = "Osc3 Decay", .unit = "ms", .value = 200.0f, .max = 3000.0f },
        { .label = "Osc3 Sustain", .unit = "%", .value = 100.0f },
        { .label = "Osc1 Attack", .unit = "ms", .value = 5.0f, .max = 3000.0f },
        { .label = "Osc2 Attack", .unit = "ms", .value = 5.0f, .max = 3000.0f },
        { .label = "Osc3 Attack", .unit = "ms", .value = 5.0f, .max = 3000.0f },
        // Dedicated Pitch Env for Kicks
        { .label = "Pitch Env Start", .unit = "st", .value = 0.0f, .max = 64.0f },
        { .label = "Pitch Env Decay", .unit = "ms", .value = 20.0f, .max = 500.0f },
        { .label = "Pitch Env Mode", .value = 0.0f, .max = 1.0f }, // 0:Off, 1:On
    };

    Param& osc1W = params[0];
    Param& osc1C = params[1];
    Param& osc1F = params[2];
    Param& osc1L = params[3];
    Param& osc2W = params[4];
    Param& osc2C = params[5];
    Param& osc2L = params[6];
    Param& osc2M = params[7];
    Param& osc2FM = params[8];
    Param& osc3W = params[9];
    Param& osc3C = params[10];
    Param& osc3L = params[11];
    Param& osc3M = params[12];
    Param& osc3FM = params[13];
    Param& sbO = params[14];
    Param& sbL = params[15];
    Param& sbW = params[16];
    Param& amA = params[17];
    Param& amD = params[18];
    Param& amS = params[19];
    Param& amR = params[20];
    Param& amV = params[21];
    Param& flC = params[22];
    Param& flR = params[23];
    Param& flT = params[24];
    Param& flE = params[25];
    Param& feS = params[26];
    Param& feR = params[27];
    Param& lf1T = params[28];
    Param& lf1R = params[29];
    Param& lf1P = params[30];
    Param& lf1F = params[31];
    Param& lf1A = params[32];
    Param& lf2T = params[33];
    Param& lf2R = params[34];
    Param& lf2P = params[35];
    Param& lf2F = params[36];
    Param& lf2A = params[37];
    Param& drv = params[38];
    Param& drvT = params[39];
    Param& bitC = params[40];
    Param& deci = params[41];
    Param& gld = params[42];
    Param& o1D = params[43];
    Param& o1S = params[44];
    Param& o2D = params[45];
    Param& o2S = params[46];
    Param& o3D = params[47];
    Param& o3S = params[48];
    Param& o1A = params[49];
    Param& o2A = params[50];
    Param& o3A = params[51];
    Param& pES = params[52];
    Param& pED = params[53];
    Param& pEM = params[54];

    SynthAI(float sr, float* = nullptr, float* = nullptr)
        : EngineBase(Synth, "SynthAI V3.2", params)
    {
        sampleRate = (sr > 0) ? sr : 44100.0f;
        inv = 1.0f / sampleRate;
        init();
    }

    void noteOnImpl(uint8_t note, float vel)
    {
        velocity = vel;
        targetNote = (float)note;
        if (!gateOpen || gld.value < 1.0f) currentNote = targetNote;
        gateOpen = true;
        ampEnvelope.st = filterEnvelope.st = 1;
        oscEnvelopes[0].st = oscEnvelopes[1].st = oscEnvelopes[2].st = 1;
        pitchEnvVal = 1.0f; // Reset Pitch Env
        for (int i = 0; i < 2; ++i) {
            int type = (int)(i == 0 ? lf1T.value : lf2T.value);
            if (type >= 4) {
                lfoPhase[i] = 0.0f;
                lfoDone[i] = false;
            }
            if (type == 12) lfoSHValue[i] = fastNoise();
        }
    }

    void noteOffImpl(uint8_t note)
    {
        if ((float)note == targetNote) {
            gateOpen = false;
            if (ampEnvelope.st > 0) ampEnvelope.st = 4;
            if (filterEnvelope.st > 0) filterEnvelope.st = 4;
            for (int i = 0; i < 3; ++i)
                if (oscEnvelopes[i].st > 0) oscEnvelopes[i].st = 4;
        }
    }

    float sampleImpl()
    {
        if (ampEnvelope.st == 0 && !gateOpen) return 0.0f;

        // Pitch Envelope Processing (Linear decay)
        if (pEM.value > 0.5f && pitchEnvVal > 0.0f) {
            float pDecayStep = 1.0f / (sampleRate * std::max(1.0f, pED.value) * 0.001f);
            pitchEnvVal -= pDecayStep;
            if (pitchEnvVal < 0.0f) pitchEnvVal = 0.0f;
        } else if (pEM.value < 0.5f) {
            pitchEnvVal = 0.0f;
        }

        float lfoVals[2];
        for (int i = 0; i < 2; ++i) {
            float rate = (i == 0 ? lf1R.value : lf2R.value);
            int type = (int)(i == 0 ? lf1T.value : lf2T.value);
            if (!lfoDone[i]) {
                lfoPhase[i] += rate * inv;
                if (lfoPhase[i] >= 1.0f) {
                    if (type >= 8 && type <= 11) {
                        lfoPhase[i] = 1.0f;
                        lfoDone[i] = true;
                    } else lfoPhase[i] -= 1.0f;
                    if (type == 12) lfoSHValue[i] = fastNoise();
                }
            }
            lfoVals[i] = lfoWaveSelect(i, lfoPhase[i], type);
        }

        if (gld.value > 1.0f) {
            float c = Math::exp(-1.0f / (sampleRate * gld.value * 0.001f));
            currentNote = targetNote + c * (currentNote - targetNote);
        } else currentNote = targetNote;

        float ampEnvVal = adsrTick(ampEnvelope, amA.value, amD.value, amS.value * 0.01f, amR.value);
        float fltEnvVal = adsrTick(filterEnvelope, 10.0f, 300.0f, feS.value * 0.01f, feR.value);

        float o1Env = adsrTick(oscEnvelopes[0], o1A.value, o1D.value, o1S.value * 0.01f, amR.value);
        float o2Env = adsrTick(oscEnvelopes[1], o2A.value, o2D.value, o2S.value * 0.01f, amR.value);
        float o3Env = adsrTick(oscEnvelopes[2], o3A.value, o3D.value, o3S.value * 0.01f, amR.value);

        // Calculate pitch with dedicated Pitch Env
        float pitchMod = currentNote + (lfoVals[0] * lf1P.value) + (lfoVals[1] * lf2P.value) + (pitchEnvVal * pES.value);
        float baseFreq = 440.0f * std::pow(2.0f, (pitchMod - 69.0f) / 12.0f);

        float s3 = waveSelect(ph[2], (int)osc3W.value);
        ph[2] += (baseFreq * std::pow(2.0f, osc3C.value / 12.0f) * osc3M.value) * inv;
        if (ph[2] >= 1.0f) ph[2] -= 1.0f;

        float s2 = waveSelect(ph[1] + (s3 * osc3FM.value * 0.01f), (int)osc2W.value);
        ph[1] += (baseFreq * std::pow(2.0f, osc2C.value / 12.0f) * osc2M.value) * inv;
        if (ph[1] >= 1.0f) ph[1] -= 1.0f;

        float s1 = waveSelect(ph[0] + (s2 * osc2FM.value * 0.01f), (int)osc1W.value);
        ph[0] += (baseFreq * std::pow(2.0f, (osc1C.value + osc1F.value * 0.01f) / 12.0f)) * inv;
        if (ph[0] >= 1.0f) ph[0] -= 1.0f;

        float sig = (s1 * o1Env * osc1L.value * 0.01f) + (s2 * o2Env * osc2L.value * 0.01f) + (s3 * o3Env * osc3L.value * 0.01f);
        float subFreq = baseFreq * 0.5f * (sbO.value > 1.5f ? 0.5f : 1.0f);
        float subSig = (sbW.value < 0.5f) ? (phSub < 0.5f ? 1.0f : -1.0f) : (2.0f * phSub - 1.0f);
        phSub += subFreq * inv;
        if (phSub >= 1.0f) phSub -= 1.0f;
        sig += subSig * (sbL.value * 0.01f);

        float cutoff = CLAMP(flC.value * 0.01f + (fltEnvVal * flE.value * 0.01f) + (lfoVals[1] * lf2F.value * 0.01f) + (lfoVals[0] * lf1F.value * 0.01f), 0.01f, 0.99f);
        filter1.setCutoff(cutoff * cutoff);
        filter1.setResonance(flR.value * 0.01f);
        auto fOut = filter1.process12(sig);
        if (flT.value < 1.0f) sig = fOut.lp;
        else if (flT.value < 2.0f) sig = fOut.hp;
        else sig = fOut.bp;

        if (drv.value > 0.0f) {
            float amount = drv.value * 0.1f;
            if (drvT.value < 0.5f) sig = std::tanh(sig * (1.0f + amount));
            else sig = std::clamp(sig * (1.0f + amount), -0.8f, 0.8f);
        }

        if (bitC.value > 0.1f) {
            float steps = std::pow(2.0f, 16.0f - bitC.value);
            sig = std::round(sig * steps) / steps;
        }

        static float lastSig = 0;
        static float deciCounter = 0;
        deciCounter += (1.0f - (deci.value * 0.009f));
        if (deciCounter >= 1.0f) {
            deciCounter -= 1.0f;
            lastSig = sig;
        }
        sig = lastSig;

        float lfoAmpMod = 1.0f + (lfoVals[0] * lf1A.value * 0.01f) + (lfoVals[1] * lf2A.value * 0.01f);
        float velScale = 1.0f - (amV.value * 0.01f) + (velocity * amV.value * 0.01f);
        return std::clamp(sig * ampEnvVal * velScale * lfoAmpMod * 0.5f, -1.0f, 1.0f);
    }
};