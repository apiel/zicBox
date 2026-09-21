#pragma once

#include "audio/engines/DriftSynth1.h"
#include "audio/engines/EngineBase.h"
#include "helpers/clamp.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>

class WaveEngine : public EngineBase<WaveEngine> {
public:
    DriftSynth1 synth;

    char modTypeNameDisplay[32] = "ENV Cutoff";

    // 14 Parameters declared matching exact count of addParam calls
    Param params[14];

    // --- Potentiometer Parameters (8 Params - Excluded from Encoder Menu) ---
    Param& pitch = addParam({ .key = "pitch", .label = "Pitch", .value = 36.0f, .min = 24.0f, .max = 72.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.pitch.set(val); } });
    Param& waveform = addParam({ .key = "waveform", .label = "Wave", .value = 0.3f, .min = 0.0f, .max = 1.0f, .step = 0.01f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.waveform.set(val); } });
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .value = 0.4f, .min = 0.02f, .max = 0.98f, .step = 0.01f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.cutoff.set(val); } });
    Param& resonance = addParam({ .key = "resonance", .label = "Reso", .value = 0.3f, .min = 0.0f, .max = 0.95f, .step = 0.01f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.resonance.set(val); } });
    Param& release = addParam({ .key = "release", .label = "Release", .unit = "ms", .value = 250.0f, .min = 10.0f, .max = 2000.0f, .step = 10.0f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.release.set(val); } });
    Param& envAmt = addParam({ .key = "envAmt", .label = "Env Amt", .value = 0.4f, .min = 0.0f, .max = 1.0f, .step = 0.01f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.envAmt.set(val); } });
    Param& filterMorph = addParam({ .key = "filterMorph", .label = "Filt Morph", .value = 0.0f, .min = 0.0f, .max = 1.0f, .step = 0.01f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.filterMorph.set(val); } });
    Param& crushFm = addParam({ .key = "crushFm", .label = "Crsh / FM", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.crushFm.set(val); } });

    // --- Encoder Menu Parameters (6 Params) ---
    Param& modType = addParam({ .key = "modType", .label = "Mod Type", .string = modTypeNameDisplay, .value = 0.0f, .min = 0.0f, .max = 15.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
                                    auto* s = (WaveEngine*)ctx;
                                    s->synth.modType.set(val);
                                    int idx = std::clamp((int)std::round(val), 0, DriftSynth1::TOTAL_MOD_TYPES - 1);
                                    strncpy(s->modTypeNameDisplay, DriftSynth1::modMatrix[idx].name, sizeof(s->modTypeNameDisplay) - 1);
                                } });
    Param& modDepth = addParam({ .key = "modDepth", .label = "Mod Depth", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.modDepth.set(val); } });
    Param& modSpeed = addParam({ .key = "modSpeed", .label = "Mod Speed", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.modSpeed.set(val); } });
    Param& delaySend = addParam({ .key = "delaySend", .label = "Dly Send", .unit = "%", .value = 20.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.delaySend.set(val); } });
    Param& masterVol = addParam({ .key = "vol", .label = "Volume", .unit = "%", .value = 80.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& bpmParam = addParam({ .key = "bpm", .label = "BPM", .value = 120.0f, .min = 40.0f, .max = 240.0f, .step = 1.0f });

    float sampleRate = 44100.0f;
    double sampleCounter = 1e9; // Force immediate first trigger on start
    uint32_t stepIndex = 0;
    bool isPlaying = true;
    bool isExternalClock = false;
    uint32_t midiClockCount = 0;

    static constexpr int SEQ_STEPS = 16;
    inline static const int noteOffsets[SEQ_STEPS] = { 0, 12, 7, 12, 3, 7, 10, 12, 0, 12, 7, 3, 5, 7, 10, 12 };

    WaveEngine(float sr = 44100.0f)
        : EngineBase(Synth, "zicModWave", params)
        , synth(sr)
        , sampleRate(sr)
    {
        syncSynthParams();
    }

    void resetClock()
    {
        sampleCounter = 1e9;
        stepIndex = 0;
        midiClockCount = 0;
    }

    void syncSynthParams()
    {
        synth.pitch.set(pitch.value);
        synth.waveform.set(waveform.value);
        synth.cutoff.set(cutoff.value);
        synth.resonance.set(resonance.value);
        synth.release.set(release.value);
        synth.envAmt.set(envAmt.value);
        synth.filterMorph.set(filterMorph.value);
        synth.crushFm.set(crushFm.value);
        synth.modType.set(modType.value);
        synth.modDepth.set(modDepth.value);
        synth.modSpeed.set(modSpeed.value);
        synth.delaySend.set(delaySend.value);

        int idx = std::clamp((int)std::round(modType.value), 0, DriftSynth1::TOTAL_MOD_TYPES - 1);
        strncpy(modTypeNameDisplay, DriftSynth1::modMatrix[idx].name, sizeof(modTypeNameDisplay) - 1);
    }

    void trigger(float note = -1.0f)
    {
        synth.trigger(note >= 0.0f ? note : pitch.value);
    }

    void onMidiClockPulse()
    {
        midiClockCount++;
        if (midiClockCount >= 6) { // 24 PPQN -> 6 ticks per 16th step
            midiClockCount = 0;
            stepIndex = (stepIndex + 1) % SEQ_STEPS;
            synth.trigger(pitch.value + noteOffsets[stepIndex]);
        }
    }

    void noteOnImpl(uint8_t note, float velocity)
    {
        synth.noteOn(note, velocity);
    }

    void noteOffImpl(uint8_t note)
    {
        synth.noteOff(note);
    }

    float sampleImpl()
    {
        if (isPlaying && !isExternalClock) {
            double bpm = std::clamp((double)bpmParam.value, 40.0, 240.0);
            double samplesPerStep = (sampleRate * 60.0) / (bpm * 4.0); // 16th note steps
            sampleCounter += 1.0;
            if (sampleCounter >= samplesPerStep) {
                sampleCounter -= samplesPerStep;
                stepIndex = (stepIndex + 1) % SEQ_STEPS;
                synth.trigger(pitch.value + noteOffsets[stepIndex]);
            }
        }

        float out = synth.sample();
        return out * (masterVol.value * 0.01f);
    }

    float drawImpl(float x)
    {
        return synth.draw(x);
    }
};
