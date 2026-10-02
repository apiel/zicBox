#pragma once

#include "esp32/src/ModWaveSynth.h"
#include "audio/engines/EngineBase.h"
#include "helpers/clamp.h"
#include "MasterFX.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>

class WaveEngine : public EngineBase<WaveEngine> {
public:
    ModWaveSynth synth;

    char modTypeNameDisplay[32] = "ENV Cutoff";

    // 15 Parameters declared matching exact count of addParam calls
    Param params[15];

    // --- Potentiometer Parameters (8 Params - Excluded from Encoder Menu) ---
    Param& pitch = addParam({ .key = "pitch", .label = "Pitch", .value = 36.0f, .min = 24.0f, .max = 72.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.pitchVal = val; } });
    Param& waveform = addParam({ .key = "waveform", .label = "Wave", .value = 0.3f, .min = 0.0f, .max = 1.0f, .step = 0.01f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.waveVal = val; } });
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .value = 0.4f, .min = 0.02f, .max = 0.98f, .step = 0.01f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.cutoffVal = val; } });
    Param& resonance = addParam({ .key = "resonance", .label = "Reso", .value = 0.3f, .min = 0.0f, .max = 0.95f, .step = 0.01f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.resVal = val; } });
    Param& ringMod = addParam({ .key = "ringMod", .label = "Ring Mod", .value = 0.0f, .min = 0.0f, .max = 1.0f, .step = 0.01f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.ringModVal = val; } });
    Param& release = addParam({ .key = "release", .label = "Release", .unit = "ms", .value = 250.0f, .min = 10.0f, .max = 2000.0f, .step = 10.0f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.releaseMs = val; } });
    Param& envAmt = addParam({ .key = "envAmt", .label = "Env Amt", .value = 0.4f, .min = 0.0f, .max = 1.0f, .step = 0.01f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.envAmtVal = val; } });
    Param& crushFm = addParam({ .key = "crushFm", .label = "Mod FX", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.crushFmVal = val; } });

    // --- Encoder Menu Parameters ---
    Param& modType = addParam({ .key = "modType", .label = "Mod Type", .string = modTypeNameDisplay, .value = 0.0f, .min = 0.0f, .max = 15.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
                                    auto* s = (WaveEngine*)ctx;
                                    s->synth.modTypeIdx = (int)std::round(val);
                                    int idx = std::clamp(s->synth.modTypeIdx, 0, ModWaveSynth::TOTAL_MOD_TYPES - 1);
                                    strncpy(s->modTypeNameDisplay, ModWaveSynth::modMatrix[idx].name, sizeof(s->modTypeNameDisplay) - 1);
                                } });
    Param& fmRatioParam = addParam({ .key = "fmRatio", .label = "FM Ratio", .value = 2.0f, .min = 0.5f, .max = 8.0f, .step = 0.5f, .onUpdate = [](void* ctx, float val) { ((WaveEngine*)ctx)->synth.fmRatioVal = val; } });
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
    
    // --- Rhythm Patterns & Arp Definitions ---
    static constexpr int TOTAL_RHYTHM_PATTERNS = 6;
    inline static const char* RHYTHM_NAMES[TOTAL_RHYTHM_PATTERNS] = {
        "1/4 Notes",
        "1/8 Notes",
        "1/16 Notes",
        "Offbeats",
        "Syncopated",
        "Acid Bass"
    };

    inline static const bool RHYTHM_STEPS[TOTAL_RHYTHM_PATTERNS][16] = {
        { 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0 }, // 1/4 Notes
        { 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0 }, // 1/8 Notes
        { 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1 }, // 1/16 Notes
        { 0,0,1,0, 0,0,1,0, 0,0,1,0, 0,0,1,0 }, // Offbeats
        { 1,0,0,1, 0,0,1,0, 0,1,0,0, 1,0,1,0 }, // Syncopated
        { 1,0,0,1, 1,0,0,1, 1,0,0,1, 1,0,1,1 }  // Acid Bass
    };

    static constexpr int TOTAL_ARP_MODES = 7;
    inline static const char* ARP_NAMES[TOTAL_ARP_MODES] = {
        "Root Only",
        "Octave Up",
        "Oct Up/Down",
        "Root + 5th",
        "Minor Triad",
        "Major Triad",
        "Pentatonic"
    };

    struct ArpDefinition {
        int length;
        int offsets[8];
    };

    inline static const ArpDefinition ARP_DEFINITIONS[TOTAL_ARP_MODES] = {
        { 1, { 0 } },                        // Root Only
        { 2, { 0, 12 } },                    // Octave Up
        { 4, { 0, 12, 0, -12 } },            // Oct Up/Down
        { 2, { 0, 7 } },                     // Root + 5th
        { 4, { 0, 3, 7, 12 } },              // Minor Triad
        { 4, { 0, 4, 7, 12 } },              // Major Triad
        { 6, { 0, 3, 5, 7, 10, 12 } }         // Pentatonic
    };

    bool rhythmMask[SEQ_STEPS] = { 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0 };
    int activeNoteOffsets[SEQ_STEPS] = { 0 };
    int rhythmPatternIdx = 0;
    int arpModeIdx = 0;

    void updateSequence(int rhythmIdx, int arpIdx)
    {
        rhythmPatternIdx = std::clamp(rhythmIdx, 0, TOTAL_RHYTHM_PATTERNS - 1);
        arpModeIdx = std::clamp(arpIdx, 0, TOTAL_ARP_MODES - 1);

        const auto& rhythm = RHYTHM_STEPS[rhythmPatternIdx];
        const auto& arp = ARP_DEFINITIONS[arpModeIdx];

        int arpHit = 0;
        for (int i = 0; i < SEQ_STEPS; ++i) {
            rhythmMask[i] = rhythm[i];
            if (rhythm[i]) {
                activeNoteOffsets[i] = arp.offsets[arpHit % arp.length];
                arpHit++;
            } else {
                activeNoteOffsets[i] = 0;
            }
        }
    }

    MasterFX masterFX;

    WaveEngine(float sr = 44100.0f)
        : EngineBase(Synth, "ModWave", params)
        , synth(sr)
        , sampleRate(sr)
    {
        syncSynthParams();
        updateSequence(0, 0);
    }

    ~WaveEngine() = default;

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
        synth.ringModVal = ringMod.value;
        synth.releaseMs = release.value;
        synth.envAmtVal = envAmt.value;
        synth.crushFmVal = crushFm.value;
        synth.fmRatioVal = fmRatioParam.value;
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
            if (rhythmMask[stepIndex]) {
                synth.trigger(pitch.value + activeNoteOffsets[stepIndex]);
            }
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
                if (rhythmMask[stepIndex]) {
                    synth.trigger(pitch.value + activeNoteOffsets[stepIndex]);
                }
            }
        }

        float drySample = synth.sample();
        return masterFX.process(drySample, delaySend.value);
    }

    float sampleImplActive(IEngine& targetEngine)
    {
        if (isPlaying && !isExternalClock) {
            double bpm = std::clamp((double)bpmParam.value, 40.0, 240.0);
            double samplesPerStep = (sampleRate * 60.0) / (bpm * 4.0); // 16th note steps
            sampleCounter += 1.0;
            if (sampleCounter >= samplesPerStep) {
                sampleCounter -= samplesPerStep;
                stepIndex = (stepIndex + 1) % SEQ_STEPS;
                if (rhythmMask[stepIndex]) {
                    targetEngine.noteOn(pitch.value + activeNoteOffsets[stepIndex], 1.0f);
                }
            }
        }

        return targetEngine.sample();
    }

    float drawImpl(float x)
    {
        return synth.draw(x);
    }
};
