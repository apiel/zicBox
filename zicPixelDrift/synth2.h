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

class Synth2 : public EngineBase<Synth2> {
private:
    float sampleRate = 44100.0f;
    float sampleRateDiv = 1.0f / 44100.0f;

    float phase1 = 0.0f;
    float phase2 = 0.0f;
    float phase3 = 0.0f;
    float subPhase = 0.0f;

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

    Param params[12];

    // Page 1: Drone Core & Chord Spread
    Param& pitch = addParam({ .key = "pitch", .label = "Pitch", .unit = "", .value = 36.0f, .min = 24.0f, .max = 72.0f, .step = 1.0f });
    Param& chord = addParam({ .key = "chord", .label = "Chord", .unit = "", .value = 1.0f, .min = 0.0f, .max = 5.0f, .step = 1.0f }); // 0:Uni, 1:5th, 2:Oct, 3:Maj7, 4:Min7, 5:Sus4
    Param& wtSelect = addParam({ .key = "wtSelect", .label = "Wavetable", .string = wtName, .value = 0.0f, .min = 0.0f, .max = 0.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
                                     auto* s = (Synth2*)ctx;
                                     int i = (int)val;
                                     s->wt.open(i, false);
                                     strncpy(s->wtName, s->wt.fileBrowser.getFileWithoutExtension(i).c_str(), sizeof(s->wtName) - 1); }, .graph = [](void* ctx, float val) {
                                     auto* s = (Synth2*)ctx;
                                     return *s->wt.sample(&val); }, .stringToFloatFn = [](void* ctx, const char* valStr) { auto s = (Synth2*)ctx; return (float)s->wt.find(std::string(valStr) + ".wav"); } });
    Param& wavetable = addParam({ .key = "wtMorph", .label = "Morph", .value = 1.0f, .min = 1.0f, .max = 64.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
                                   auto* s = (Synth2*)ctx;
                                   s->wt.morph((int)val);
                               } });

    // Page 2: Atmosphere & Filter
    Param& subDrone = addParam({ .key = "subDrone", .label = "Sub Drone", .unit = "%", .value = 40.0f, .min = 0.0f, .max = 100.0f, .step = 2.0f });
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .unit = "", .value = 0.25f, .min = 0.02f, .max = 0.98f, .step = 0.02f });
    Param& resonance = addParam({ .key = "resonance", .label = "Reso", .unit = "", .value = 0.5f, .min = 0.0f, .max = 0.95f, .step = 0.02f });
    Param& delaySend = addParam({ .key = "delaySend", .label = "Dly Send", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 2.0f });

    // Page 3: Generative Drift & Fades
    Param& attack = addParam({ .key = "attack", .label = "Attack", .unit = "ms", .value = 800.0f, .min = 0.0f, .max = 4000.0f, .step = 10.0f });
    Param& release = addParam({ .key = "release", .label = "Release", .unit = "ms", .value = 2500.0f, .min = 10.0f, .max = 8000.0f, .step = 10.0f });
    Param& driftSpeed = addParam({ .key = "driftSpeed", .label = "Drift Spd", .unit = "%", .value = 30.0f, .min = 0.0f, .max = 100.0f, .step = 2.0f });
    Param& driftDepth = addParam({ .key = "driftDepth", .label = "Drift Dep", .unit = "%", .value = 40.0f, .min = 0.0f, .max = 100.0f, .step = 2.0f });

    Synth2(float sr = 44100.0f)
        : EngineBase(Synth, "Synth2", params)
        , sampleRate(sr)
        , sampleRateDiv(1.0f / sr)
    {
        wtSelect.max = std::max(0.0f, (float)(wt.fileBrowser.count - 1));
        wt.open(0, true);
        strncpy(wtName, wt.fileBrowser.getFileWithoutExtension(0).c_str(), sizeof(wtName) - 1);
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

        // Generative Chaos LFO Drift
        float driftHz = 0.02f + (pct(driftSpeed) * 1.5f);
        chaosLfoPhase += driftHz * sampleRateDiv;
        if (chaosLfoPhase >= 1.0f) {
            chaosLfoPhase -= 1.0f;
            chaosVal = nextNoise() * 0.5f;
        }

        float driftMod = chaosVal * pct(driftDepth);

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

        float basePitch = currentFreq * (1.0f + driftMod * 0.03f);
        float f1 = basePitch;
        float f2 = basePitch * ratio2;
        float f3 = basePitch * ratio3;

        float wtSampleCount = (float)wt.sampleCount;
        phase1 += f1 * sampleRateDiv * wtSampleCount;
        if (phase1 >= wtSampleCount) phase1 -= wtSampleCount;
        phase2 += f2 * sampleRateDiv * wtSampleCount;
        if (phase2 >= wtSampleCount) phase2 -= wtSampleCount;
        phase3 += f3 * sampleRateDiv * wtSampleCount;
        if (phase3 >= wtSampleCount) phase3 -= wtSampleCount;

        subPhase += (basePitch * 0.5f) * sampleRateDiv;
        if (subPhase > 1.0f) subPhase -= 1.0f;

        // Lush Evolving Wavetable Timbres from Wavetable Folder
        float finalMorph = std::clamp(wavetable.value + driftMod * 16.0f, 1.0f, 64.0f);
        wt.morph((int)finalMorph);

        float s1 = wtRead(wt, phase1);
        float s2 = wtRead(wt, phase2);
        float s3 = wtRead(wt, phase3);

        float oscMix = (s1 + s2 + s3) * 0.333333f;

        // Sub Drone Bass Layer
        float subSine = Math::fastSin2(PI_X2 * subPhase);
        oscMix += subSine * pct(subDrone) * 0.6f;

        // Resonant State-Variable Filter
        float cutMod = std::clamp(cutoff.value + driftMod * 0.15f, 0.02f, 0.98f);
        filter.setCutoff(cutMod);
        filter.setResonance(resonance.value);
        FilterSVF::Data& svf = filter.process12(oscMix);

        return svf.lp * ampEnv;
    }
};
