#pragma once

#include "audio/engines/DriftSynth1.h"
#include "audio/engines/DriftWavetable.h"
#include "audio/engines/DrumGeneric.h"
#include "audio/sequencer/Step.h"
#include <atomic>
#include <mutex>
#include <vector>

#ifndef SEQ_STEPS_MINI
#define SEQ_STEPS_MINI 16
#endif

struct TribeDrums {
    DrumGeneric snare { 44100.0f, nullptr, nullptr };
    DrumGeneric hhClosed { 44100.0f, nullptr, nullptr };
    DrumGeneric hhOpen { 44100.0f, nullptr, nullptr };
    DrumGeneric clap { 44100.0f, nullptr, nullptr };

    float snareVol = 0.85f;
    float hhVol = 0.75f;
    float clapVol = 0.80f;

    TribeDrums()
    {
        // 0. Snare / Mental Rim Patch
        snare.mix.value = 0.0f;
        snare.character.value = 20.0f;
        snare.baseFrequency.value = 185.0f;
        snare.hiClapDuration.value = 100.0f;
        snare.bodyDuration.value = 90.0f;
        snare.snapTone.value = 30.0f;

        // 1. Closed HiHat Patch (Driving 16th Tekno HH)
        hhClosed.mix.value = 100.0f;
        hhClosed.character.value = -100.0f;
        hhClosed.hiClapDuration.value = 45.0f;
        hhClosed.hiTightness.value = 70.0f;
        hhClosed.hiInharmonic.value = 50.0f;
        hhClosed.snapTone.value = 50.0f;

        // 2. Open HiHat Patch (Offbeat Cymbal Accent)
        hhOpen.mix.value = 100.0f;
        hhOpen.character.value = -100.0f;
        hhOpen.hiClapDuration.value = 220.0f;
        hhOpen.hiTightness.value = 30.0f;
        hhOpen.hiInharmonic.value = 45.0f;
        hhOpen.snapTone.value = 40.0f;

        // 3. Clap / Mental Zap Patch
        clap.mix.value = 100.0f;
        clap.character.value = 100.0f;
        clap.hiClapDuration.value = 180.0f;
        clap.clapPunch.value = 70.0f;
        clap.clapNoiseClr.value = 75.0f;
        clap.snapTone.value = 20.0f;
    }

    void noteOn(int voice, uint8_t note, float vel)
    {
        if (voice == 0) snare.noteOn(note, vel);
        else if (voice == 1) hhClosed.noteOn(note, vel);
        else if (voice == 2) hhOpen.noteOn(note, vel);
        else if (voice == 3) clap.noteOn(note, vel);
    }

    float sample()
    {
        float s0 = snare.sample() * snareVol;
        float s1 = hhClosed.sample() * hhVol;
        float s2 = hhOpen.sample() * hhVol;
        float s3 = clap.sample() * clapVol;
        return s0 + s1 + s2 + s3;
    }
};

struct DrumTrackState {
    TribeDrums drums;
    Step sequence[4][SEQ_STEPS_MINI]; // 4 rows: Snare, CHH, OHH, Clap
    bool rowEnabled[4] = { true, true, true, true };
    bool isMuted = false;
    float volume = 0.85f;

    DrumTrackState()
    {
        for (int r = 0; r < 4; ++r) {
            for (int i = 0; i < SEQ_STEPS_MINI; ++i) {
                sequence[r][i].active = false;
                sequence[r][i].note = 60;
                sequence[r][i].velocity = 0.9f;
            }
        }
        initDefaultPattern();
    }

    void initDefaultPattern()
    {
        // Row 0: Snare on steps 4, 12
        sequence[0][4].active = true;
        sequence[0][12].active = true;

        // Row 1: Closed HH on offbeats & 16ths
        for (int i = 0; i < SEQ_STEPS_MINI; i++) {
            if (i % 2 == 1) sequence[1][i].active = true;
        }

        // Row 2: Open HH on 2, 6, 10, 14
        sequence[2][2].active = true;
        sequence[2][6].active = true;
        sequence[2][10].active = true;
        sequence[2][14].active = true;

        // Row 3: Clap on 12
        sequence[3][12].active = true;
    }
};

template <typename EngineType>
struct SynthTrackState {
    EngineType engine;
    Step sequence[SEQ_STEPS_MINI];
    bool isMuted = false;
    float volume = 0.80f;

    SynthTrackState(uint8_t defaultNote = 48, bool defaultPattern = false)
        : engine(44100.0f)
    {
        for (int i = 0; i < SEQ_STEPS_MINI; i++) {
            sequence[i].active = defaultPattern ? (i % 4 == 2) : (i % 8 == 0);
            sequence[i].note = defaultNote + ((i % 4 == 2) ? 7 : 0);
            sequence[i].velocity = 0.85f;
        }
    }
};

struct StudioState {
    DrumTrackState trackDrums;
    SynthTrackState<DriftSynth1> trackSynth1 { 48, true };   // Synth 1 (Lead/Bass)
    SynthTrackState<DriftWavetable> trackSynth2 { 36, false }; // Synth 2 (Wavetable Sub/Pad)

    std::mutex audioMutex;

    std::atomic<bool> isPlaying { true };
    std::atomic<float> bpm { 140.0f };
    std::atomic<int> currentStep { 0 };

    std::atomic<bool> drumPulseTrigger { false };
    std::atomic<bool> synth1PulseTrigger { false };
    std::atomic<bool> synth2PulseTrigger { false };

    float sampleRate = 44100.0f;
    uint32_t stepCounter = 0;
    uint32_t samplesPerStep = 0;

    void updateClock()
    {
        samplesPerStep = static_cast<uint32_t>((sampleRate * 60.0f) / (bpm.load() * 4.0f));
    }
};

extern StudioState studio;
