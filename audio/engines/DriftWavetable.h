#pragma once

#ifndef AUDIO_FOLDER
#include "host/constants.h"
#endif

#include "audio/Wavetable.h"
#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/linearInterpolation.h"
#include "audio/utils/math.h"
#include "helpers/clamp.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

class DriftWavetable : public EngineBase<DriftWavetable> {
public:
    enum ModSource {
        SRC_ENV,
        SRC_LFO_TRI,
        SRC_LFO_SAW,
        SRC_LFO_SH,
        SRC_DRIFT
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

    static constexpr int TOTAL_MOD_TYPES = 17;
    inline static const ModRouting modMatrix[TOTAL_MOD_TYPES] = {
        { "ENV Cutoff", SRC_ENV, DST_FILTER },
        { "ENV Pitch", SRC_ENV, DST_PITCH },
        { "ENV Morph", SRC_ENV, DST_MORPH },
        { "LFO Tri Cut", SRC_LFO_TRI, DST_FILTER },
        { "LFO Tri Pit", SRC_LFO_TRI, DST_PITCH },
        { "LFO Tri Morph", SRC_LFO_TRI, DST_MORPH },
        { "LFO Tri Lvl", SRC_LFO_TRI, DST_LEVEL },
        { "LFO Saw Cut", SRC_LFO_SAW, DST_FILTER },
        { "LFO Saw Pit", SRC_LFO_SAW, DST_PITCH },
        { "LFO Saw Morph", SRC_LFO_SAW, DST_MORPH },
        { "LFO S&H Cut", SRC_LFO_SH, DST_FILTER },
        { "LFO S&H Pit", SRC_LFO_SH, DST_PITCH },
        { "LFO S&H Morph", SRC_LFO_SH, DST_MORPH },
        { "Drift Cut", SRC_DRIFT, DST_FILTER },
        { "Drift Pit", SRC_DRIFT, DST_PITCH },
        { "Drift Morph", SRC_DRIFT, DST_MORPH },
        { "Drift Level", SRC_DRIFT, DST_LEVEL }
    };

private:
    float sampleRate = 44100.0f;
    float sampleRateDiv = 1.0f / 44100.0f;

    float phase1 = 0.0f;
    float phase2 = 0.0f;
    float phase3 = 0.0f;

    float lfoPhase = 0.0f;
    uint32_t shCounter = 0;
    float shValue = 0.0f;

    float chaosLfoPhase = 0.0f;
    float chaosVal = 0.0f;

    float ampEnv = 0.0f;
    bool isTriggered = false;

    float currentFreq = 65.41f; // C2
    float targetFreq = 65.41f;

    FilterSVF filter;

    float lerp(float a, float b, float t) { return a + t * (b - a); }

    // Fast noise LCG
    uint32_t noiseState = 54321;
    float nextNoise()
    {
        noiseState = noiseState * 196314165 + 907633389;
        return (float)int32_t(noiseState) / 2147483648.f;
    }

    float wtRead(Wavetable& wt, float pos)
    {
        float sc = (float)wt.sampleCount;
        pos = pos - std::floor(pos / sc) * sc;
        return linearInterpolationAbsolute(pos, wt.sampleCount, wt.samples());
    }

public:
    Wavetable wt;
    char wtName[64] = "---";
    char chordName[32] = "Fifth";
    char modTypeName[32] = "ENV Cutoff";

    Param params[12];

    // Page 1: Chord & Wavetable
    Param& pitch = addParam({ .key = "pitch", .label = "Pitch", .value = 44.0f, .min = 24.0f, .max = 72.0f, .step = 1.0f });
    Param& chord = addParam({ .key = "chord", .label = "Chord", .string = chordName, .value = 1.0f, .min = 0.0f, .max = 5.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
                                  auto* s = (DriftWavetable*)ctx;
                                  int cIdx = (int)std::round(val);
                                  const char* cStr = "Unison";
                                  if (cIdx == 1) cStr = "Fifth";
                                  else if (cIdx == 2) cStr = "Octave";
                                  else if (cIdx == 3) cStr = "Maj 7th";
                                  else if (cIdx == 4) cStr = "Min 7th";
                                  else if (cIdx == 5) cStr = "Sus 4";
                                  strncpy(s->chordName, cStr, sizeof(s->chordName) - 1);
                              } }); // 0:Uni, 1:5th, 2:Oct, 3:Maj7, 4:Min7, 5:Sus4
    Param& wtSelect = addParam({ .key = "wtSelect", .label = "Wavetable", .string = wtName, .value = 20.0f, .min = 0.0f, .max = 0.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
                                     auto* s = (DriftWavetable*)ctx;
                                     int i = (int)val;
                                     s->wt.open(i, false);
                                     strncpy(s->wtName, s->wt.fileBrowser.getFileWithoutExtension(i).c_str(), sizeof(s->wtName) - 1); }, .graph = [](void* ctx, float val) {
                                     auto* s = (DriftWavetable*)ctx;
                                     return s->wt.readMorph(s->wavetable.value, val * s->wt.sampleCount); }, .stringToFloatFn = [](void* ctx, const char* valStr) { auto s = (DriftWavetable*)ctx; return (float)s->wt.find(std::string(valStr) + ".wav"); } });
    Param& wavetable = addParam({ .key = "wtMorph", .label = "Morph", .value = 1.0f, .min = 1.0f, .max = 64.0f, .step = 0.1f, .onUpdate = [](void* ctx, float val) {
                                   auto* s = (DriftWavetable*)ctx;
                                   s->wt.morph((int)val);
                               }, .graph = [](void* ctx, float val) {
                                   auto* s = (DriftWavetable*)ctx;
                                   return s->wt.readMorph(s->wavetable.value, val * s->wt.sampleCount);
                               } });

    // Page 2: Filter & Envelope
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .value = 0.5f, .min = 0.02f, .max = 0.98f, .step = 0.01f });
    Param& resonance = addParam({ .key = "resonance", .label = "Reso", .value = 0.95f, .min = 0.0f, .max = 0.95f, .step = 0.01f });
    Param& attack = addParam({ .key = "attack", .label = "Attack", .unit = "ms", .value = 10.0f, .min = 0.0f, .max = 4000.0f, .step = 10.0f });
    Param& release = addParam({ .key = "release", .label = "Release", .unit = "ms", .value = 250.0f, .min = 10.0f, .max = 8000.0f, .step = 10.0f });

    // Page 3: Modulation & Delay Send
    Param& modType = addParam({ .key = "modType", .label = "Mod Type", .string = modTypeName, .value = 0.0f, .min = 0.0f, .max = 16.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
                                    auto* s = (DriftWavetable*)ctx;
                                    int idx = std::clamp((int)std::round(val), 0, TOTAL_MOD_TYPES - 1);
                                    strncpy(s->modTypeName, modMatrix[idx].name, sizeof(s->modTypeName) - 1);
                                } });
    Param& modDepth = addParam({ .key = "modDepth", .label = "Mod Depth", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 1.0f });
    Param& modSpeed = addParam({ .key = "modSpeed", .label = "Mod Speed", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& delaySend = addParam({ .key = "delaySend", .label = "Dly Send", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });

    DriftWavetable(float sr = 44100.0f)
        : EngineBase(Synth, "DriftWavetable", params)
        , sampleRate(sr)
        , sampleRateDiv(1.0f / sr)
    {
        wtSelect.max = std::max(0.0f, (float)(wt.fileBrowser.count - 1));
        wt.open(0, true);
        strncpy(wtName, wt.fileBrowser.getFileWithoutExtension(0).c_str(), sizeof(wtName) - 1);
        chord.set(1.0f);
        modType.set(0.0f);
    }

    void trigger(float midiNote = -1.0f)
    {
        if (midiNote >= 0.0f) pitch.value = midiNote;
        targetFreq = 440.0f * std::pow(2.0f, (pitch.value - 69.0f) / 12.0f);
        currentFreq = targetFreq;
        isTriggered = true;
    }

    void noteOnImpl(uint8_t note, float velocity)
    {
        trigger((float)note);
    }

    float sampleImpl()
    {
        // Smooth Attack & Release Envelope
        float attackStep = 1.0f / (sampleRate * (attack.value * 0.001f));
        float releaseCoeff = std::exp(-1.0f / (sampleRate * (release.value * 0.001f)));

        if (isTriggered) {
            ampEnv += attackStep;
            if (ampEnv >= 1.0f) {
                ampEnv = 1.0f;
                isTriggered = false;
            }
        } else {
            ampEnv *= releaseCoeff;
        }

        if (ampEnv < 0.0001f) return 0.0f;

        // LFO Phase & Rate matching Synth1
        float lfoHz = 0.05f + (pct(modSpeed) * pct(modSpeed) * 39.95f);
        lfoPhase += lfoHz * sampleRateDiv;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

        // Generative Chaos LFO Drift
        float driftHz = 0.02f + (pct(modSpeed) * 1.5f);
        chaosLfoPhase += driftHz * sampleRateDiv;
        if (chaosLfoPhase >= 1.0f) {
            chaosLfoPhase -= 1.0f;
            chaosVal = nextNoise();
        }

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
        case SRC_DRIFT:
            srcVal = chaosVal;
            break;
        }

        float modulationAmount = srcVal * (modDepth.value * 0.01f);

        float finalCutoff = cutoff.value;
        float finalPitchInterval = 0.0f;
        float finalMorph = wavetable.value;
        float levelMod = 1.0f;

        if (currentRoute.dest == DST_FILTER) {
            finalCutoff = std::clamp(cutoff.value + modulationAmount, 0.01f, 0.99f);
        } else if (currentRoute.dest == DST_PITCH) {
            finalPitchInterval = modulationAmount * 12.0f;
        } else if (currentRoute.dest == DST_MORPH) {
            finalMorph = std::clamp(wavetable.value + modulationAmount * 31.0f, 1.0f, 64.0f);
        } else if (currentRoute.dest == DST_LEVEL) {
            levelMod = std::clamp(1.0f + modulationAmount, 0.0f, 2.0f);
        }

        // Calculate Chord Frequencies (Osc 1, Osc 2, Osc 3)
        float ratio2 = 1.002f; // Unison detune
        float ratio3 = 1.0f;

        int chordType = (int)std::round(chord.value);
        switch (chordType) {
        case 1: ratio3 = 1.4983f; break; // Fifth
        case 2: ratio3 = 2.0000f; break; // Octave
        case 3: ratio2 = 1.2599f; ratio3 = 1.8877f; break; // Major 7th
        case 4: ratio2 = 1.1892f; ratio3 = 1.7818f; break; // Minor 7th
        case 5: ratio2 = 1.3348f; ratio3 = 1.4983f; break; // Sus4
        default: ratio2 = 1.003f; ratio3 = 0.997f; break; // Unison detune spread
        }

        float effectiveFreq = currentFreq * std::pow(2.0f, finalPitchInterval / 12.0f);
        float f1 = effectiveFreq;
        float f2 = effectiveFreq * ratio2;
        float f3 = effectiveFreq * ratio3;

        float wtSampleCount = (float)wt.sampleCount;
        phase1 += f1 * sampleRateDiv * wtSampleCount;
        if (phase1 >= wtSampleCount) phase1 -= wtSampleCount;
        phase2 += f2 * sampleRateDiv * wtSampleCount;
        if (phase2 >= wtSampleCount) phase2 -= wtSampleCount;
        phase3 += f3 * sampleRateDiv * wtSampleCount;
        if (phase3 >= wtSampleCount) phase3 -= wtSampleCount;

        // Wavetable Morphing with 2D Frame Interpolation
        float m = std::clamp(finalMorph - 1.0f, 0.0f, (float)(ZIC_WAVETABLE_WAVEFORMS_COUNT - 1));
        int idx1 = (int)m;
        int idx2 = (idx1 < ZIC_WAVETABLE_WAVEFORMS_COUNT - 1) ? idx1 + 1 : idx1;
        float frac = m - (float)idx1;

        float s1 = wt.readMorph(idx1, idx2, frac, phase1);
        float s2 = wt.readMorph(idx1, idx2, frac, phase2);
        float s3 = wt.readMorph(idx1, idx2, frac, phase3);

        float oscMix = (s1 + s2 + s3) * 0.333333f;

        // Resonant State-Variable Filter
        filter.setCutoff(finalCutoff);
        filter.setResonance(resonance.value);
        FilterSVF::Data& svf = filter.process12(oscMix);

        return svf.lp * ampEnv * levelMod;
    }
};
