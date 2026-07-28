#pragma once

#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

class Synth1 : public EngineBase<Synth1> {
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
        DST_MORPH,
        DST_LEVEL
    };

    struct ModRouting {
        const char* name;
        ModSource source;
        ModDest dest;
    };

    static constexpr int TOTAL_MOD_TYPES = 12;
    inline static const ModRouting modMatrix[TOTAL_MOD_TYPES] = {
        { "ENV Cutoff", SRC_ENV, DST_FILTER },
        { "ENV Pitch", SRC_ENV, DST_PITCH },
        { "ENV Wave", SRC_ENV, DST_MORPH },
        { "LFO Tri Cut", SRC_LFO_TRI, DST_FILTER },
        { "LFO Tri Pit", SRC_LFO_TRI, DST_PITCH },
        { "LFO Tri Wave", SRC_LFO_TRI, DST_MORPH },
        { "LFO Tri Lvl", SRC_LFO_TRI, DST_LEVEL },
        { "LFO Saw Cut", SRC_LFO_SAW, DST_FILTER },
        { "LFO Saw Pit", SRC_LFO_SAW, DST_PITCH },
        { "LFO Saw Wave", SRC_LFO_SAW, DST_MORPH },
        { "LFO S&H Cut", SRC_LFO_SH, DST_FILTER },
        { "LFO S&H Pit", SRC_LFO_SH, DST_PITCH }
    };

private:
    float sampleRate = 44100.0f;
    float sampleRateDiv = 1.0f / 44100.0f;

    float phase = 0.0f;
    float lfoPhase = 0.0f;
    float ampEnv = 0.0f;
    float currentFreq = 110.0f;
    float targetFreq = 110.0f;

    FilterSVF filter;
    uint32_t noiseState = 98765;
    uint32_t shCounter = 0;
    float shValue = 0.0f;

    float nextNoise()
    {
        noiseState = noiseState * 196314165 + 907633389;
        return (float)int32_t(noiseState) / 2147483648.f;
    }

    float lerp(float a, float b, float t) { return a + t * (b - a); }

public:
    Param params[12];

    // Page 1: Tone & Filter
    Param& pitch = addParam({ .key = "pitch", .label = "Pitch", .unit = "", .value = 36.0f, .min = 24.0f, .max = 72.0f, .step = 1.0f });
    Param& waveform = addParam({ .key = "waveform", .label = "Wave", .unit = "", .value = 0.3f, .min = 0.0f, .max = 1.0f, .step = 0.02f });
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .unit = "", .value = 0.2f, .min = 0.02f, .max = 0.98f, .step = 0.02f });
    Param& resonance = addParam({ .key = "resonance", .label = "Reso", .unit = "", .value = 1.0f, .min = 0.0f, .max = 0.95f, .step = 0.02f });

    // Page 2: Envelope & Filter Morph
    Param& release = addParam({ .key = "release", .label = "Release", .unit = "ms", .value = 110.0f, .min = 10.0f, .max = 2000.0f, .step = 10.0f });
    Param& envAmt = addParam({ .key = "envAmt", .label = "Env Amt", .unit = "", .value = 0.2f, .min = 0.0f, .max = 1.0f, .step = 0.02f });
    Param& filterMorph = addParam({ .key = "filterMorph", .label = "Filt Morph", .unit = "", .value = 0.0f, .min = 0.0f, .max = 1.0f, .step = 0.02f });
    Param& delaySend = addParam({ .key = "delaySend", .label = "Dly Send", .unit = "%", .value = 30.0f, .min = 0.0f, .max = 100.0f, .step = 2.0f });

    // Page 3: Modulation & Synth Mix
    Param& modType = addParam({ .key = "modType", .label = "Mod Type", .unit = "", .value = 0.0f, .min = 0.0f, .max = 11.0f, .step = 1.0f });
    Param& modDepth = addParam({ .key = "modDepth", .label = "Mod Depth", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 2.0f });
    Param& modSpeed = addParam({ .key = "modSpeed", .label = "Mod Speed", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 2.0f });

    Synth1(float sr = 44100.0f)
        : EngineBase(Synth, "Synth1", params)
        , sampleRate(sr)
        , sampleRateDiv(1.0f / sr)
    {
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

        // LFO Phase & Rate matching drop2.h
        float lfoHz = 0.05f + (pct(modSpeed) * pct(modSpeed) * 39.95f);
        lfoPhase += lfoHz * sampleRateDiv;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

        // Route selection matching drop2.h
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
        float finalWaveform = waveform.value;
        float levelMod = 1.0f;

        if (currentRoute.dest == DST_FILTER) {
            finalCutoff = std::clamp(cutoff.value + modulationAmount, 0.01f, 0.99f);
        } else if (currentRoute.dest == DST_PITCH) {
            finalPitchInterval = modulationAmount * 12.0f;
        } else if (currentRoute.dest == DST_MORPH) {
            finalWaveform = std::clamp(waveform.value + modulationAmount, 0.0f, 1.0f);
        } else if (currentRoute.dest == DST_LEVEL) {
            levelMod = std::clamp(1.0f + modulationAmount, 0.0f, 2.0f);
        }

        float effectiveFreq = currentFreq * std::pow(2.0f, finalPitchInterval / 12.0f);

        phase += effectiveFreq * sampleRateDiv;
        if (phase > 1.0f) phase -= 1.0f;

        // 4-Waveform Morphing: Tri (0%) -> Saw (33%) -> Sq (66%) -> Noise (100%)
        float ph = phase;
        float tri = 2.0f * std::abs(2.0f * (ph - std::floor(ph + 0.5f))) - 1.0f;
        float saw = 2.0f * ph - 1.0f;
        float sq = (ph < 0.5f) ? 0.8f : -0.8f;
        float ns = nextNoise();

        float wf = finalWaveform;
        float oscSig = 0.0f;
        if (wf < 0.333f) {
            oscSig = lerp(tri, saw, wf * 3.0f);
        } else if (wf < 0.666f) {
            oscSig = lerp(saw, sq, (wf - 0.333f) * 3.0f);
        } else {
            oscSig = lerp(sq, ns, (wf - 0.666f) * 3.0f);
        }

        // Filter SVF processing
        float cutEnv = std::clamp(finalCutoff + (ampEnv * envAmt.value * 0.5f), 0.02f, 0.98f);
        filter.setCutoff(cutEnv);
        filter.setResonance(resonance.value);
        FilterSVF::Data& svf = filter.process12(oscSig);

        // Filter Morphing: LP (0%) -> BP (50%) -> HP (100%)
        float outSig = 0.0f;
        float fMorph = filterMorph.value;
        if (fMorph < 0.5f) {
            outSig = lerp(svf.lp, svf.bp, fMorph * 2.0f);
        } else {
            outSig = lerp(svf.bp, svf.hp, (fMorph - 0.5f) * 2.0f);
        }

        return outSig * ampEnv * levelMod;
    }
};
