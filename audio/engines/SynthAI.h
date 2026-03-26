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
    ADSR ampEnvelope, filterEnvelope;

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

    static constexpr const char* WAVE_NAMES[5] = { "Sine", "Saw", "Square", "Triangle", "Noise" };
    static constexpr const char* FILTER_NAMES[4] = { "LP12", "HP12", "BP12", "LP24" };

    float fastNoise()
    {
        seed = 214013 * seed + 2531011;
        return ((float)((seed >> 16) & 0x7FFF) / 32768.0f) * 2.0f - 1.0f;
    }

    float waveBandLimited(float ph_in, float dt, int type)
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

    float adsrTick(ADSR& envelope, float attackMs, float decayMs, float sustainLevel, float releaseMs)
    {
        float attackStep = attackMs < 1.0f ? 1.0f : 1.0f / (sampleRate * attackMs * 0.001f);
        float decayCoeff = Math::exp(-1.0f / (sampleRate * std::max(1.0f, decayMs) * 0.001f));
        float releaseCoeff = releaseMs < 1.0f ? 0.0f : Math::exp(-1.0f / (sampleRate * releaseMs * 0.001f));

        switch (envelope.st) {
        case 1: // Attack
            envelope.v += attackStep;
            if (envelope.v >= 1.0f) {
                envelope.v = 1.0f;
                envelope.st = 2;
            }
            break;
        case 2: // Decay
            envelope.v = sustainLevel + (envelope.v - sustainLevel) * decayCoeff;
            if (std::abs(envelope.v - sustainLevel) < 0.001f) {
                envelope.v = sustainLevel;
                envelope.st = 3;
            }
            break;
        case 3: // Sustain
            envelope.v = sustainLevel;
            break;
        case 4: // Release
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
    Param params[56] = {
        { .label = "Osc1 Wave", .string = osc1WaveName, .value = 1.0f, .max = 4.0f, .onUpdate = [](void* c, float v) { strncpy(((SynthAI*)c)->osc1WaveName, WAVE_NAMES[(int)v], 15); } },
        { .label = "Osc1 Coarse", .unit = "st", .value = 0.0f, .min = -24.0f, .max = 24.0f },
        { .label = "Osc1 Fine", .unit = "ct", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Osc1 Level", .unit = "%", .value = 100.0f },
        { .label = "Osc2 Wave", .string = osc2WaveName, .value = 0.0f, .max = 4.0f, .onUpdate = [](void* c, float v) { strncpy(((SynthAI*)c)->osc2WaveName, WAVE_NAMES[(int)v], 15); } },
        { .label = "Osc2 Coarse", .unit = "st", .value = 0.0f, .min = -24.0f, .max = 24.0f },
        { .label = "Osc2 Level", .unit = "%", .value = 0.0f },
        { .label = "Osc2 Multiplier", .unit = "x", .value = 1.0f, .min = 0.25f, .max = 16.0f },
        { .label = "Osc2 FM Amount", .unit = "%", .value = 0.0f },
        { .label = "Osc3 Wave", .string = osc3WaveName, .value = 0.0f, .max = 4.0f, .onUpdate = [](void* c, float v) { strncpy(((SynthAI*)c)->osc3WaveName, WAVE_NAMES[(int)v], 15); } },
        { .label = "Osc3 Coarse", .unit = "st", .value = -12.0f, .min = -24.0f, .max = 24.0f },
        { .label = "Osc3 Level", .unit = "%", .value = 0.0f },
        { .label = "Osc3 Multiplier", .unit = "x", .value = 1.0f, .min = 0.25f, .max = 16.0f },
        { .label = "Osc3 FM Amount", .unit = "%", .value = 0.0f },
        { .label = "Sub Octave", .value = 1.0f, .max = 2.0f },
        { .label = "Sub Level", .unit = "%", .value = 0.0f },
        { .label = "Sub Waveform", .value = 0.0f, .max = 1.0f },
        { .label = "Noise Mix", .unit = "%", .value = 0.0f },
        { .label = "Noise Color", .unit = "%", .value = 50.0f },
        { .label = "Amp Attack", .unit = "ms", .value = 5.0f, .max = 3000.0f },
        { .label = "Amp Decay", .unit = "ms", .value = 200.0f, .max = 3000.0f },
        { .label = "Amp Sustain", .unit = "%", .value = 70.0f },
        { .label = "Amp Release", .unit = "ms", .value = 300.0f, .max = 5000.0f },
        { .label = "Amp Velocity", .unit = "%", .value = 50.0f },
        { .label = "Filter Cutoff", .unit = "%", .value = 80.0f },
        { .label = "Filter Resonance", .unit = "%", .value = 10.0f },
        { .label = "Filter Type", .string = filter1TypeName, .value = 0.0f, .max = 3.0f, .onUpdate = [](void* c, float v) { strncpy(((SynthAI*)c)->filter1TypeName, FILTER_NAMES[(int)v], 15); } },
        { .label = "Filter Env Amount", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Filter LFO1 Amount", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Filter Key Track", .unit = "%", .value = 20.0f },
        { .label = "Filter Velocity", .unit = "%", .value = 20.0f },
        { .label = "Filter Attack", .unit = "ms", .value = 10.0f, .max = 3000.0f },
        { .label = "Filter Decay", .unit = "ms", .value = 300.0f, .max = 3000.0f },
        { .label = "Filter Sustain", .unit = "%", .value = 0.0f },
        { .label = "Filter Release", .unit = "ms", .value = 300.0f, .max = 5000.0f },
        { .label = "Filter2 Cutoff", .unit = "%", .value = 100.0f },
        { .label = "Filter2 Resonance", .unit = "%", .value = 0.0f },
        { .label = "Filter2 Type", .value = 1.0f, .max = 3.0f },
        { .label = "Filter2 Env Amount", .unit = "%", .value = 0.0f },
        { .label = "Pitch Env Amount", .unit = "st", .value = 0.0f, .min = -48.0f, .max = 48.0f },
        { .label = "Pitch Env Decay", .unit = "ms", .value = 100.0f, .max = 1000.0f },
        { .label = "Mod Env Attack", .unit = "ms", .value = 5.0f, .max = 1000.0f },
        { .label = "Mod Env Decay", .unit = "ms", .value = 200.0f, .max = 2000.0f },
        { .label = "Mod Env To FM", .unit = "%", .value = 0.0f },
        { .label = "LFO1 Rate", .unit = "Hz", .value = 5.0f, .max = 50.0f },
        { .label = "LFO1 Wave", .value = 0.0f, .max = 4.0f },
        { .label = "LFO1 To Pitch", .unit = "st", .value = 0.0f, .max = 12.0f },
        { .label = "LFO1 Fade", .unit = "ms", .value = 0.0f, .max = 2000.0f },
        { .label = "LFO2 Rate", .unit = "Hz", .value = 0.5f, .max = 50.0f },
        { .label = "LFO2 To FM", .unit = "%", .value = 0.0f },
        { .label = "LFO2 To Filter2", .unit = "%", .value = 0.0f },
        { .label = "Drive", .unit = "%", .value = 0.0f },
        { .label = "Drive Type", .value = 0.0f, .max = 2.0f },
        { .label = "Bitcrush", .unit = "bits", .value = 0.0f, .max = 8.0f },
        { .label = "Decimator", .unit = "%", .value = 0.0f },
        { .label = "Glide", .unit = "ms", .value = 0.0f, .max = 1000.0f }
    };

    // Named References (Expanded for clarity)
    Param& osc1Wave = params[0];
    Param& osc1Coarse = params[1];
    Param& osc1Fine = params[2];
    Param& osc1Level = params[3];
    Param& osc2Wave = params[4];
    Param& osc2Coarse = params[5];
    Param& osc2Level = params[6];
    Param& osc2Multiplier = params[7];
    Param& osc2FMAmount = params[8];
    Param& osc3Wave = params[9];
    Param& osc3Coarse = params[10];
    Param& osc3Level = params[11];
    Param& osc3Multiplier = params[12];
    Param& osc3FMAmount = params[13];
    Param& subOctave = params[14];
    Param& subLevel = params[15];
    Param& subWaveform = params[16];
    Param& noiseMix = params[17];
    Param& ampAttack = params[19];
    Param& ampDecay = params[20];
    Param& ampSustain = params[21];
    Param& ampRelease = params[22];
    Param& ampVelocity = params[23];
    Param& filter1Cutoff = params[24];
    Param& filter1Resonance = params[25];
    Param& filter1Type = params[26];
    Param& filter1EnvAmount = params[27];
    Param& filterAttack = params[31];
    Param& filterDecay = params[32];
    Param& filterSustain = params[33];
    Param& filterRelease = params[34];
    Param& glide = params[55];

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
        if (!gateOpen || glide.value < 1.0f) currentNote = targetNote;
        gateOpen = true;
        ampEnvelope.st = filterEnvelope.st = 1; // Explicitly set to ATTACK
    }

    void noteOffImpl(uint8_t note)
    {
        if ((float)note == targetNote) {
            gateOpen = false;
            if (ampEnvelope.st > 0) ampEnvelope.st = 4; // Explicitly set to RELEASE
            if (filterEnvelope.st > 0) filterEnvelope.st = 4;
        }
    }

    float sampleImpl()
    {
        if (ampEnvelope.st == 0 && !gateOpen) return 0.0f;

        // Glide
        if (glide.value > 1.0f) {
            float c = Math::exp(-1.0f / (sampleRate * glide.value * 0.001f));
            currentNote = targetNote + c * (currentNote - targetNote);
        } else {
            currentNote = targetNote;
        }

        float ampEnvVal = adsrTick(ampEnvelope, ampAttack.value, ampDecay.value, ampSustain.value * 0.01f, ampRelease.value);
        float fltEnvVal = adsrTick(filterEnvelope, filterAttack.value, filterDecay.value, filterSustain.value * 0.01f, filterRelease.value);

        float baseFreq = 440.0f * std::pow(2.0f, (currentNote - 69.0f) / 12.0f);

        // Osc 3 -> Osc 2 -> Osc 1 Chain
        float s3 = waveBandLimited(ph[2], (baseFreq * osc3Multiplier.value) * inv, (int)osc3Wave.value);
        ph[2] += (baseFreq * std::pow(2.0f, osc3Coarse.value / 12.0f) * osc3Multiplier.value) * inv;
        if (ph[2] >= 1.0f) ph[2] -= 1.0f;

        float s2 = waveBandLimited(ph[1] + (s3 * osc3FMAmount.value * 0.01f), (baseFreq * osc2Multiplier.value) * inv, (int)osc2Wave.value);
        ph[1] += (baseFreq * std::pow(2.0f, osc2Coarse.value / 12.0f) * osc2Multiplier.value) * inv;
        if (ph[1] >= 1.0f) ph[1] -= 1.0f;

        float s1 = waveBandLimited(ph[0] + (s2 * osc2FMAmount.value * 0.01f), baseFreq * inv, (int)osc1Wave.value);
        ph[0] += (baseFreq * std::pow(2.0f, (osc1Coarse.value + osc1Fine.value * 0.01f) / 12.0f)) * inv;
        if (ph[0] >= 1.0f) ph[0] -= 1.0f;

        // Mixer
        float sig = s1 * (osc1Level.value * 0.01f) + s2 * (osc2Level.value * 0.01f) + s3 * (osc3Level.value * 0.01f);

        // Sub
        float subFreq = baseFreq * 0.5f * (subOctave.value > 1.5f ? 0.5f : 1.0f);
        float subSig = (subWaveform.value < 0.5f) ? (phSub < 0.5f ? 1.0f : -1.0f) : (2.0f * phSub - 1.0f);
        phSub += subFreq * inv;
        if (phSub >= 1.0f) phSub -= 1.0f;
        sig += subSig * (subLevel.value * 0.01f);

        // Noise
        sig += fastNoise() * (noiseMix.value * 0.01f);

        // Filter 1 (State Variable)
        float cutoff = CLAMP(filter1Cutoff.value * 0.01f + (fltEnvVal * filter1EnvAmount.value * 0.01f), 0.01f, 0.99f);
        filter1.setCutoff(cutoff * cutoff);
        filter1.setResonance(filter1Resonance.value * 0.01f);
        auto filterOut = filter1.process12(sig);

        if (filter1Type.value < 1.0f) sig = filterOut.lp;
        else if (filter1Type.value < 2.0f) sig = filterOut.hp;
        else sig = filterOut.bp;

        float velScaling = 1.0f - (ampVelocity.value * 0.01f) + (velocity * ampVelocity.value * 0.01f);
        return sig * ampEnvVal * velScaling * 0.5f;
    }
};