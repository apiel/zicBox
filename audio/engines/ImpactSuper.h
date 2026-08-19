#pragma once

#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

class ImpactSuper : public EngineBase<ImpactSuper> {
public:
    enum ModSource {
        SRC_ENV,
        SRC_LFO_TRI,
        SRC_LFO_SAW,
        SRC_LFO_SH
    };

    enum ModDest {
        DST_FILTER,
        DST_PITCH,
        DST_DETUNE,
        DST_WAVE,
        DST_LEVEL
    };

    struct ModRouting {
        const char* name;
        ModSource source;
        ModDest dest;
    };

    static constexpr int TOTAL_MOD_TYPES = 16;
    inline static const ModRouting modMatrix[TOTAL_MOD_TYPES] = {
        { "ENV Cutoff", SRC_ENV, DST_FILTER },
        { "ENV Pitch", SRC_ENV, DST_PITCH },
        { "ENV Detune", SRC_ENV, DST_DETUNE },
        { "ENV Wave", SRC_ENV, DST_WAVE },
        { "LFO Tri Cut", SRC_LFO_TRI, DST_FILTER },
        { "LFO Tri Pit", SRC_LFO_TRI, DST_PITCH },
        { "LFO Tri Det", SRC_LFO_TRI, DST_DETUNE },
        { "LFO Tri Wave", SRC_LFO_TRI, DST_WAVE },
        { "LFO Tri Lvl", SRC_LFO_TRI, DST_LEVEL },
        { "LFO Saw Cut", SRC_LFO_SAW, DST_FILTER },
        { "LFO Saw Pit", SRC_LFO_SAW, DST_PITCH },
        { "LFO Saw Det", SRC_LFO_SAW, DST_DETUNE },
        { "LFO Saw Wave", SRC_LFO_SAW, DST_WAVE },
        { "LFO S&H Cut", SRC_LFO_SH, DST_FILTER },
        { "LFO S&H Pit", SRC_LFO_SH, DST_PITCH },
        { "LFO S&H Det", SRC_LFO_SH, DST_DETUNE }
    };

private:
    float sampleRate = 44100.0f;
    float sampleRateDiv = 1.0f / 44100.0f;

    // 7 Unison Oscillators
    static constexpr int NUM_OSCS = 7;
    float oscPhases[NUM_OSCS] = { 0.0f };

    float subPhase = 0.0f;
    float ampEnv = 0.0f;
    float currentFreq = 110.0f;
    float targetFreq = 110.0f;
    float lfoPhase = 0.0f;

    FilterSVF filter;
    uint32_t noiseState = 123456789;
    uint32_t shCounter = 0;
    float shValue = 0.0f;

    float nextNoise()
    {
        noiseState = noiseState * 196314165 + 907633389;
        return (float)int32_t(noiseState) / 2147483648.f;
    }

    static float lerp(float a, float b, float t) { return a + t * (b - a); }

public:
    char modTypeName[32] = "ENV Cutoff";
    Param params[12];

    // Page 1: Oscillator & Unison Tone
    Param& pitch = addParam({ .key = "pitch", .label = "Pitch", .unit = "", .value = 36.0f, .min = 24.0f, .max = 72.0f, .step = 1.0f });
    Param& detune = addParam({ .key = "detune", .label = "Detune", .unit = "%", .value = 40.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& subOct = addParam({ .key = "subOct", .label = "Sub Level", .unit = "%", .value = 30.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& waveform = addParam({ .key = "waveform", .label = "Wave Morph", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });

    // Page 2: Filter & Envelope
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .unit = "", .value = 0.35f, .min = 0.02f, .max = 0.98f, .step = 0.01f });
    Param& resonance = addParam({ .key = "resonance", .label = "Reso", .unit = "", .value = 0.4f, .min = 0.0f, .max = 0.95f, .step = 0.01f });
    Param& envAmt = addParam({ .key = "envAmt", .label = "Env Amt", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& release = addParam({ .key = "release", .label = "Release", .unit = "ms", .value = 250.0f, .min = 10.0f, .max = 3000.0f, .step = 10.0f });

    // Page 3: Modulation Matrix & Space
    Param& modType = addParam({ .key = "modType", .label = "Mod Type", .string = modTypeName, .value = 0.0f, .min = 0.0f, .max = 15.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
                                    auto* s = static_cast<ImpactSuper*>(ctx);
                                    int idx = std::clamp((int)std::round(val), 0, TOTAL_MOD_TYPES - 1);
                                    strncpy(s->modTypeName, modMatrix[idx].name, sizeof(s->modTypeName) - 1);
                                } });
    Param& modDepth = addParam({ .key = "modDepth", .label = "Mod Depth", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 1.0f });
    Param& modSpeed = addParam({ .key = "modSpeed", .label = "Mod Speed", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& delaySend = addParam({ .key = "delaySend", .label = "Dly Send", .unit = "%", .value = 25.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });

    ImpactSuper(float sr = 44100.0f)
        : EngineBase(Synth, "ImpactSuper", params)
        , sampleRate(sr)
        , sampleRateDiv(1.0f / sr)
    {
        modType.set(0.0f);
    }

    void trigger(float midiNote = -1.0f)
    {
        if (midiNote >= 0.0f) pitch.value = midiNote;
        targetFreq = 440.0f * std::pow(2.0f, (pitch.value - 69.0f) / 12.0f);
        currentFreq = targetFreq;
        ampEnv = 1.0f;
    }

    void noteOnImpl(uint8_t note, float velocity)
    {
        trigger((float)note);
    }

    float sampleImpl()
    {
        if (ampEnv < 0.0001f) return 0.0f;

        float relCoeff = std::exp(-1.0f / (sampleRate * (release.value * 0.001f)));
        ampEnv *= relCoeff;

        // LFO Phase & Frequency
        float modS = modSpeed.value * 0.01f;
        float lfoHz = 0.05f + (modS * modS * 39.95f);
        lfoPhase += lfoHz * sampleRateDiv;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

        // Route selection
        int routeIdx = std::clamp((int)std::round(modType.value), 0, TOTAL_MOD_TYPES - 1);
        ModRouting currentRoute = modMatrix[routeIdx];

        float srcVal = 0.0f;
        switch (currentRoute.source) {
        case SRC_ENV:
            srcVal = ampEnv;
            break;
        case SRC_LFO_TRI:
            srcVal = (lfoPhase < 0.5f) ? (4.0f * lfoPhase - 1.0f) : (3.0f - 4.0f * lfoPhase);
            break;
        case SRC_LFO_SAW:
            srcVal = 2.0f * lfoPhase - 1.0f;
            break;
        case SRC_LFO_SH: {
            uint32_t samplesPerHold = std::max((uint32_t)1, (uint32_t)(sampleRate / std::max(0.1f, lfoHz)));
            if (++shCounter >= samplesPerHold) {
                shCounter = 0;
                shValue = nextNoise();
            }
            srcVal = shValue;
            break;
        }
        }

        float modulationAmount = srcVal * (modDepth.value * 0.01f);

        float finalCutoff = cutoff.value;
        float finalPitchInterval = 0.0f;
        float finalDetune = detune.value * 0.01f;
        float finalWaveform = waveform.value * 0.01f;
        float levelMod = 1.0f;

        if (currentRoute.dest == DST_FILTER) {
            finalCutoff = std::clamp(cutoff.value + modulationAmount * 0.5f, 0.02f, 0.98f);
        } else if (currentRoute.dest == DST_PITCH) {
            finalPitchInterval = modulationAmount * 12.0f;
        } else if (currentRoute.dest == DST_DETUNE) {
            finalDetune = std::clamp((detune.value * 0.01f) + modulationAmount, 0.0f, 1.0f);
        } else if (currentRoute.dest == DST_WAVE) {
            finalWaveform = std::clamp((waveform.value * 0.01f) + modulationAmount, 0.0f, 1.0f);
        } else if (currentRoute.dest == DST_LEVEL) {
            levelMod = std::clamp(1.0f + modulationAmount, 0.0f, 2.0f);
        }

        float effectiveFreq = currentFreq * std::pow(2.0f, finalPitchInterval / 12.0f);

        static constexpr float detuneRatios[NUM_OSCS] = {
            1.0000f, 1.0042f, 0.9958f, 1.0091f, 0.9912f, 1.0150f, 0.9850f
        };

        float oscMix = 0.0f;
        for (int i = 0; i < NUM_OSCS; ++i) {
            float ratio = 1.0f + (detuneRatios[i] - 1.0f) * finalDetune;
            float freq = effectiveFreq * ratio;

            oscPhases[i] += freq * sampleRateDiv;
            if (oscPhases[i] >= 1.0f) oscPhases[i] -= 1.0f;

            float ph = oscPhases[i];
            float saw = 2.0f * ph - 1.0f;
            float sq = (ph < 0.5f) ? 0.8f : -0.8f;
            float oscVal = lerp(saw, sq, finalWaveform);

            oscMix += oscVal;
        }
        oscMix *= (1.0f / (1.0f + 0.4f * (NUM_OSCS - 1)));

        // Sub Oscillator (1 Octave Down)
        subPhase += (effectiveFreq * 0.5f) * sampleRateDiv;
        if (subPhase >= 1.0f) subPhase -= 1.0f;
        float subVal = (subPhase < 0.5f) ? 0.7f : -0.7f;
        float subMix = subVal * (subOct.value * 0.01f);

        float combinedSig = oscMix + subMix;

        // Filter Envelope & Processing
        float cutEnv = std::clamp(finalCutoff + (ampEnv * (envAmt.value * 0.01f) * 0.5f), 0.02f, 0.98f);
        filter.setCutoff(cutEnv);
        filter.setResonance(resonance.value);

        FilterSVF::Data& svf = filter.process12(combinedSig);
        float filteredSig = svf.lp;

        return filteredSig * ampEnv * levelMod;
    }
};
