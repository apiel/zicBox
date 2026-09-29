#pragma once

#include "esp32/src/ModWaveSynth.h"
#include "audio/engines/EngineBase.h"
#include "helpers/clamp.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>

class WaveEngine : public EngineBase<WaveEngine> {
public:
    ModWaveSynth synth;

    char modTypeNameDisplay[32] = "ENV Cutoff";

    // 14 Parameters declared matching exact count of addParam calls
    Param params[14];

    // --- Potentiometer Parameters (8 Params - Excluded from Encoder Menu) ---
    Param& pitch = addParam({ .key = "pitch", .label = "Pitch", .value = 36.0f, .min = 24.0f, .max = 72.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.pitchVal = val; } });
    Param& waveform = addParam({ .key = "waveform", .label = "Wave", .value = 0.3f, .min = 0.0f, .max = 1.0f, .step = 0.01f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.waveVal = val; } });
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .value = 0.4f, .min = 0.02f, .max = 0.98f, .step = 0.01f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.cutoffVal = val; } });
    Param& resonance = addParam({ .key = "resonance", .label = "Reso", .value = 0.3f, .min = 0.0f, .max = 0.95f, .step = 0.01f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.resVal = val; } });
    Param& release = addParam({ .key = "release", .label = "Release", .unit = "ms", .value = 250.0f, .min = 10.0f, .max = 2000.0f, .step = 10.0f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.releaseMs = val; } });
    Param& envAmt = addParam({ .key = "envAmt", .label = "Env Amt", .value = 0.4f, .min = 0.0f, .max = 1.0f, .step = 0.01f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.envAmtVal = val; } });
    Param& filterMorph = addParam({ .key = "filterMorph", .label = "Filt Morph", .value = 0.0f, .min = 0.0f, .max = 1.0f, .step = 0.01f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.filterMorphVal = val; } });
    Param& crushFm = addParam({ .key = "crushFm", .label = "Crsh / FM", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.crushFmVal = val; } });

    // --- Encoder Menu Parameters (6 Params) ---
    Param& modType = addParam({ .key = "modType", .label = "Mod Type", .string = modTypeNameDisplay, .value = 0.0f, .min = 0.0f, .max = 15.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
                                    auto* s = (WaveEngine*)ctx;
                                    s->synth.modTypeIdx = (int)std::round(val);
                                    int idx = std::clamp(s->synth.modTypeIdx, 0, ModWaveSynth::TOTAL_MOD_TYPES - 1);
                                    strncpy(s->modTypeNameDisplay, ModWaveSynth::modMatrix[idx].name, sizeof(s->modTypeNameDisplay) - 1);
                                } });
    Param& modDepth = addParam({ .key = "modDepth", .label = "Mod Depth", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.modDepthVal = val; } });
    Param& modSpeed = addParam({ .key = "modSpeed", .label = "Mod Speed", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.modSpeedVal = val; } });
    Param& delaySend = addParam({ .key = "delaySend", .label = "Dly Send", .unit = "%", .value = 20.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.delaySendVal = val; } });
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

    // --- Audio Delay DSP Buffer ---
    static constexpr int DELAY_BUF_SIZE = 8192;
    float* delayBuffer = nullptr;
    int delayWriteIdx = 0;
    float delayTimeMs = 180.0f;
    float delayFeedback = 0.50f;

    WaveEngine(float sr = 44100.0f)
        : EngineBase(Synth, "zicModWave", params)
        , synth(sr)
        , sampleRate(sr)
    {
        delayBuffer = (float*)calloc(DELAY_BUF_SIZE, sizeof(float));
        syncSynthParams();
    }

    ~WaveEngine()
    {
        if (delayBuffer) {
            free(delayBuffer);
            delayBuffer = nullptr;
        }
    }

    void resetClock()
    {
        sampleCounter = 1e9;
        stepIndex = 0;
        midiClockCount = 0;
    }

    void syncSynthParams()
    {
        synth.pitchVal = pitch.value;
        synth.waveVal = waveform.value;
        synth.cutoffVal = cutoff.value;
        synth.resVal = resonance.value;
        synth.releaseMs = release.value;
        synth.envAmtVal = envAmt.value;
        synth.filterMorphVal = filterMorph.value;
        synth.crushFmVal = crushFm.value;
        synth.modTypeIdx = (int)std::round(modType.value);
        synth.modDepthVal = modDepth.value;
        synth.modSpeedVal = modSpeed.value;
        synth.delaySendVal = delaySend.value;

        int idx = std::clamp(synth.modTypeIdx, 0, ModWaveSynth::TOTAL_MOD_TYPES - 1);
        strncpy(modTypeNameDisplay, ModWaveSynth::modMatrix[idx].name, sizeof(modTypeNameDisplay) - 1);
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

        float drySample = synth.sample();
        if (!delayBuffer) return drySample * (masterVol.value * 0.01f);

        float sendGain = std::clamp(delaySend.value * 0.01f, 0.0f, 1.0f);

        int delaySamples = (int)(delayTimeMs * 0.001f * sampleRate);
        delaySamples = std::clamp(delaySamples, 100, DELAY_BUF_SIZE - 1);

        int readIdx = (delayWriteIdx - delaySamples + DELAY_BUF_SIZE) % DELAY_BUF_SIZE;
        float wetSample = delayBuffer[readIdx];

        delayBuffer[delayWriteIdx] = drySample * sendGain + wetSample * delayFeedback;
        delayWriteIdx = (delayWriteIdx + 1) % DELAY_BUF_SIZE;

        float mixedSample = drySample + wetSample * sendGain;
        return mixedSample * (masterVol.value * 0.01f);
    }

    float drawImpl(float x)
    {
        return synth.draw(x);
    }
};
