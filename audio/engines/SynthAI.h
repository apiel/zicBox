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

    struct ADSR {
        float v = 0.0f;
        int st = 0; // 0:Idle, 1:Attack, 2:Decay, 3:Sustain, 4:Release
    };
    ADSR eAmp, eFlt;

    float currentNote = 60.0f;
    float targetNote = 60.0f;
    float velocity = 1.0f;
    bool gateOpen = false;

    FilterSVF flt1;
    uint32_t seed = 12345;

    char o1WaveName[16] = "Saw";
    char o2WaveName[16] = "Sine";
    char o3WaveName[16] = "Sine";
    char f1TypeName[16] = "LP12";

    static constexpr const char* WAVE_NAMES[5] = { "Sine", "Saw", "Square", "Triangle", "Noise" };
    static constexpr const char* FILT_NAMES[4] = { "LP12", "HP12", "BP12", "LP24" };

    float fastNoise()
    {
        seed = 214013 * seed + 2531011;
        return ((float)((seed >> 16) & 0x7FFF) / 32768.0f) * 2.0f - 1.0f;
    }

    float waveBL(float ph_in, float dt, int type)
    {
        ph_in -= std::floor(ph_in);
        switch (type) {
        case 1:
            return (2.0f * ph_in - 1.0f); // Saw
        case 2:
            return (ph_in < 0.5f) ? 1.0f : -1.0f; // Square
        case 3:
            return (ph_in < 0.5f) ? (4.0f * ph_in - 1.0f) : (3.0f - 4.0f * ph_in); // Tri
        case 4:
            return fastNoise(); // Noise
        default:
            return Math::fastSin(6.2831853f * ph_in); // Sine
        }
    }

    float adsrTick(ADSR& e, float attMs, float decMs, float susLvl, float relMs)
    {
        float aStep = attMs < 1.0f ? 1.0f : 1.0f / (sampleRate * attMs * 0.001f);
        float dCoeff = Math::exp(-1.0f / (sampleRate * std::max(1.0f, decMs) * 0.001f));
        float rCoeff = relMs < 1.0f ? 0.0f : Math::exp(-1.0f / (sampleRate * relMs * 0.001f));

        switch (e.st) {
        case 1: // Attack
            e.v += aStep;
            if (e.v >= 1.0f) {
                e.v = 1.0f;
                e.st = 2;
            }
            break;
        case 2: // Decay
            e.v = susLvl + (e.v - susLvl) * dCoeff;
            if (std::abs(e.v - susLvl) < 0.001f) {
                e.v = susLvl;
                e.st = 3;
            }
            break;
        case 3: // Sustain
            e.v = susLvl; // Stays here until noteOff
            break;
        case 4: // Release
            e.v *= rCoeff;
            if (e.v < 0.0001f) {
                e.v = 0.0f;
                e.st = 0;
            }
            break;
        }
        return e.v;
    }

public:
    Param params[56] = {
        { .label = "O1 Wave", .string = o1WaveName, .value = 1.0f, .max = 4.0f, .onUpdate = [](void* c, float v) { strncpy(((SynthAI*)c)->o1WaveName, WAVE_NAMES[(int)v], 15); } },
        { .label = "O1 Coarse", .unit = "st", .value = 0.0f, .min = -24.0f, .max = 24.0f },
        { .label = "O1 Fine", .unit = "ct", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "O1 Level", .unit = "%", .value = 100.0f },
        { .label = "O2 Wave", .string = o2WaveName, .value = 0.0f, .max = 4.0f, .onUpdate = [](void* c, float v) { strncpy(((SynthAI*)c)->o2WaveName, WAVE_NAMES[(int)v], 15); } },
        { .label = "O2 Coarse", .unit = "st", .value = 0.0f, .min = -24.0f, .max = 24.0f },
        { .label = "O2 Level", .unit = "%", .value = 0.0f },
        { .label = "O2 Mult", .unit = "x", .value = 1.0f, .min = 0.25f, .max = 16.0f },
        { .label = "O2 FM>1", .unit = "%", .value = 0.0f },
        { .label = "O3 Wave", .string = o3WaveName, .value = 0.0f, .max = 4.0f, .onUpdate = [](void* c, float v) { strncpy(((SynthAI*)c)->o3WaveName, WAVE_NAMES[(int)v], 15); } },
        { .label = "O3 Coarse", .unit = "st", .value = -12.0f, .min = -24.0f, .max = 24.0f },
        { .label = "O3 Level", .unit = "%", .value = 0.0f },
        { .label = "O3 Mult", .unit = "x", .value = 1.0f, .min = 0.25f, .max = 16.0f },
        { .label = "O3 FM>2", .unit = "%", .value = 0.0f },
        { .label = "Sub Oct", .value = 1.0f, .max = 2.0f },
        { .label = "Sub Level", .unit = "%", .value = 0.0f },
        { .label = "Sub Wave", .value = 0.0f, .max = 1.0f },
        { .label = "Noise Mix", .unit = "%", .value = 0.0f },
        { .label = "Noise Color", .unit = "%", .value = 50.0f },
        { .label = "Amp Att", .unit = "ms", .value = 5.0f, .max = 3000.0f },
        { .label = "Amp Dec", .unit = "ms", .value = 200.0f, .max = 3000.0f },
        { .label = "Amp Sus", .unit = "%", .value = 70.0f },
        { .label = "Amp Rel", .unit = "ms", .value = 300.0f, .max = 5000.0f },
        { .label = "Amp Vel>", .unit = "%", .value = 50.0f },
        { .label = "F1 Cut", .unit = "%", .value = 80.0f },
        { .label = "F1 Res", .unit = "%", .value = 10.0f },
        { .label = "F1 Type", .string = f1TypeName, .value = 0.0f, .max = 3.0f, .onUpdate = [](void* c, float v) { strncpy(((SynthAI*)c)->f1TypeName, FILT_NAMES[(int)v], 15); } },
        { .label = "F1 Env>", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "F1 LFO1>", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "F1 Key>", .unit = "%", .value = 20.0f },
        { .label = "F1 Vel>", .unit = "%", .value = 20.0f },
        { .label = "Flt Att", .unit = "ms", .value = 10.0f, .max = 3000.0f },
        { .label = "Flt Dec", .unit = "ms", .value = 300.0f, .max = 3000.0f },
        { .label = "Flt Sus", .unit = "%", .value = 0.0f },
        { .label = "Flt Rel", .unit = "ms", .value = 300.0f, .max = 5000.0f },
        { .label = "F2 Cut", .unit = "%", .value = 100.0f },
        { .label = "F2 Res", .unit = "%", .value = 0.0f },
        { .label = "F2 Type", .value = 1.0f, .max = 3.0f },
        { .label = "F2 Env>", .unit = "%", .value = 0.0f },
        { .label = "P.Env Amt", .unit = "st", .value = 0.0f, .min = -48.0f, .max = 48.0f },
        { .label = "P.Env Dec", .unit = "ms", .value = 100.0f, .max = 1000.0f },
        { .label = "M.Env Att", .unit = "ms", .value = 5.0f, .max = 1000.0f },
        { .label = "M.Env Dec", .unit = "ms", .value = 200.0f, .max = 2000.0f },
        { .label = "M.Env>FM", .unit = "%", .value = 0.0f },
        { .label = "LFO1 Rate", .unit = "Hz", .value = 5.0f, .max = 50.0f },
        { .label = "LFO1 Wave", .value = 0.0f, .max = 4.0f },
        { .label = "LFO1>Pit", .unit = "st", .value = 0.0f, .max = 12.0f },
        { .label = "LFO1 Fade", .unit = "ms", .value = 0.0f, .max = 2000.0f },
        { .label = "LFO2 Rate", .unit = "Hz", .value = 0.5f, .max = 50.0f },
        { .label = "LFO2>FM", .unit = "%", .value = 0.0f },
        { .label = "LFO2>F2", .unit = "%", .value = 0.0f },
        { .label = "Drive", .unit = "%", .value = 0.0f },
        { .label = "Drive Type", .value = 0.0f, .max = 2.0f },
        { .label = "Bitcrush", .unit = "bits", .value = 0.0f, .max = 8.0f },
        { .label = "Decimator", .unit = "%", .value = 0.0f },
        { .label = "Glide", .unit = "ms", .value = 0.0f, .max = 1000.0f }
    };

    // Named References (Crucial for maintenance)
    Param& o1W = params[0];
    Param& o1C = params[1];
    Param& o1F = params[2];
    Param& o1L = params[3];
    Param& o2W = params[4];
    Param& o2C = params[5];
    Param& o2L = params[6];
    Param& o2M = params[7];
    Param& o2FM = params[8];
    Param& o3W = params[9];
    Param& o3C = params[10];
    Param& o3L = params[11];
    Param& o3M = params[12];
    Param& o3FM = params[13];
    Param& sbO = params[14];
    Param& sbL = params[15];
    Param& sbW = params[16];
    Param& nsM = params[17];
    Param& amA = params[19];
    Param& amD = params[20];
    Param& amS = params[21];
    Param& amR = params[22];
    Param& amV = params[23];
    Param& f1C = params[24];
    Param& f1R = params[25];
    Param& f1T = params[26];
    Param& f1E = params[27];
    Param& feA = params[31];
    Param& feD = params[32];
    Param& feS = params[33];
    Param& feR = params[34];
    Param& gld = params[55];

    SynthAI(float sr, float* = nullptr, float* = nullptr)
        : EngineBase(Synth, "SynthAI V2.2", params)
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
        eAmp.st = eFlt.st = 1; // Explicitly set to ATTACK
    }

    void noteOffImpl(uint8_t note)
    {
        if ((float)note == targetNote) {
            gateOpen = false;
            if (eAmp.st > 0) eAmp.st = 4; // Explicitly set to RELEASE
            if (eFlt.st > 0) eFlt.st = 4;
        }
    }

    float sampleImpl()
    {
        if (eAmp.st == 0 && !gateOpen) return 0.0f;

        // Glide
        if (gld.value > 1.0f) {
            float c = Math::exp(-1.0f / (sampleRate * gld.value * 0.001f));
            currentNote = targetNote + c * (currentNote - targetNote);
        } else {
            currentNote = targetNote;
        }

        float ea = adsrTick(eAmp, amA.value, amD.value, amS.value * 0.01f, amR.value);
        float ef = adsrTick(eFlt, feA.value, feD.value, feS.value * 0.01f, feR.value);

        float baseF = 440.0f * std::pow(2.0f, (currentNote - 69.0f) / 12.0f);

        // Osc 3 -> Osc 2 -> Osc 1 Chain
        float s3 = waveBL(ph[2], (baseF * o3M.value) * inv, (int)o3W.value);
        ph[2] += (baseF * std::pow(2.0f, o3C.value / 12.0f) * o3M.value) * inv;
        if (ph[2] >= 1.0f) ph[2] -= 1.0f;

        float s2 = waveBL(ph[1] + (s3 * o3FM.value * 0.01f), (baseF * o2M.value) * inv, (int)o2W.value);
        ph[1] += (baseF * std::pow(2.0f, o2C.value / 12.0f) * o2M.value) * inv;
        if (ph[1] >= 1.0f) ph[1] -= 1.0f;

        float s1 = waveBL(ph[0] + (s2 * o2FM.value * 0.01f), baseF * inv, (int)o1W.value);
        ph[0] += (baseF * std::pow(2.0f, (o1C.value + o1F.value * 0.01f) / 12.0f)) * inv;
        if (ph[0] >= 1.0f) ph[0] -= 1.0f;

        // Mixer
        float sig = s1 * (o1L.value * 0.01f) + s2 * (o2L.value * 0.01f) + s3 * (o3L.value * 0.01f);

        // Sub
        float subF = baseF * 0.5f * (sbO.value > 1.5f ? 0.5f : 1.0f);
        float subS = (sbW.value < 0.5f) ? (phSub < 0.5f ? 1.0f : -1.0f) : (2.0f * phSub - 1.0f);
        phSub += subF * inv;
        if (phSub >= 1.0f) phSub -= 1.0f;
        sig += subS * (sbL.value * 0.01f);

        // Noise
        sig += fastNoise() * (nsM.value * 0.01f);

        // Filter 1 (State Variable)
        float cut1 = CLAMP(f1C.value * 0.01f + (ef * f1E.value * 0.01f), 0.01f, 0.99f);
        flt1.setCutoff(cut1 * cut1);
        flt1.setResonance(f1R.value * 0.01f);
        auto out1 = flt1.process12(sig);

        if (f1T.value < 1.0f) sig = out1.lp;
        else if (f1T.value < 2.0f) sig = out1.hp;
        else sig = out1.bp;

        float vMix = 1.0f - (amV.value * 0.01f) + (velocity * amV.value * 0.01f);
        return sig * ea * vMix * 0.5f;
    }
};