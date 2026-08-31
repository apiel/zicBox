#pragma once

#include "audio/engines/DrumGeneric.h"
#include "audio/engines/TeKKick.h"
#include "audio/engines/TeKSynth.h"
#include "audio/sequencer/Step.h"
#include <atomic>
#include <mutex>
#include <vector>

#ifndef MAX_TRACKS
#define MAX_TRACKS 3
#endif

#ifndef SEQ_STEPS_TEK
#define SEQ_STEPS_TEK 64
#endif

template <typename EngineType>
struct TrackState {
    EngineType engine;
    EngineType& kick = engine;
    EngineType& synth = engine;

    Step sequence[SEQ_STEPS_TEK];
    bool rowEnabled[4] = { true, true, true, true };
    bool isMuted = false;
    float volume = 0.85f;

    TrackState(uint8_t defaultNote = 60, bool fourOnFloor = true)
        : engine()
        , kick(engine)
        , synth(engine)
    {
        for (int i = 0; i < SEQ_STEPS_TEK; i++) {
            if (fourOnFloor) {
                sequence[i].active = (i % 4 == 0); // 4-on-the-floor kick
            } else {
                sequence[i].active = (i % 4 == 2); // Off-beat synth stabs
            }
            sequence[i].note = defaultNote;
            sequence[i].velocity = 0.9f;
        }
    }
};

struct TribeDrums {
    DrumGeneric snare { 44100.0f, nullptr, nullptr };
    DrumGeneric hhClosed { 44100.0f, nullptr, nullptr };
    DrumGeneric hhOpen { 44100.0f, nullptr, nullptr };
    DrumGeneric clap { 44100.0f, nullptr, nullptr };

    float snareVol = 0.85f;
    float hhVol = 0.75f;
    float clapVol = 0.80f;
    float drumCutoff = 0.0f; // -100 to +100 filter cutoff

    TribeDrums()
    {
        // 0. Snare / Mental Rim Patch
        snare.mix.value = 0.0f;
        snare.character.value = 20.0f;
        snare.baseFrequency.value = 185.0f;
        snare.hiClapDuration.value = 140.0f;
        snare.bodyDuration.value = 100.0f;
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
        float sum = s0 + s1 + s2 + s3;

        if (std::abs(drumCutoff) > 0.5f) {
            snare.cutoff.value = drumCutoff;
            hhClosed.cutoff.value = drumCutoff;
            hhOpen.cutoff.value = drumCutoff;
            clap.cutoff.value = drumCutoff;
        }

        return sum;
    }
};

struct Track3State {
    TribeDrums drums;

    Step sequence[SEQ_STEPS_TEK];
    Step prevSequence[SEQ_STEPS_TEK];
    bool hasPrevSequence = false;

    bool rowEnabled[4] = { true, true, true, true };
    bool isMuted = false;
    float volume = 0.80f;

    Track3State()
    {
        for (int i = 0; i < SEQ_STEPS_TEK; i++) {
            sequence[i].active = false;
            sequence[i].note = 60;
            sequence[i].velocity = 0.9f;
            prevSequence[i] = sequence[i];
        }

        initDefaultPattern();
    }

    void initDefaultPattern()
    {
        // Row 0: Snare (steps 4, 12, 14)
        sequence[4].active = true;
        sequence[12].active = true;
        sequence[14].active = true;

        // Row 1: Closed HH (driving offbeats & 16ths)
        for (int i = 0; i < 16; i++) {
            if (i % 2 == 1 || i % 4 == 2) {
                sequence[16 + i].active = true;
            }
        }

        // Row 2: Open HH (offbeats 2, 6, 10, 14)
        sequence[32 + 2].active = true;
        sequence[32 + 6].active = true;
        sequence[32 + 10].active = true;
        sequence[32 + 14].active = true;

        // Row 3: Clap (steps 4, 12)
        sequence[48 + 4].active = true;
        sequence[48 + 12].active = true;

        saveHistory();
    }

    void saveHistory()
    {
        for (int i = 0; i < SEQ_STEPS_TEK; i++) {
            prevSequence[i] = sequence[i];
        }
        hasPrevSequence = true;
    }

    void undoPattern()
    {
        if (!hasPrevSequence) return;
        for (int i = 0; i < SEQ_STEPS_TEK; i++) {
            Step temp = sequence[i];
            sequence[i] = prevSequence[i];
            prevSequence[i] = temp;
        }
    }
};

struct StudioState {
    TrackState<TeKKick> track0 { 60, true };   // Left Track: Massive Kick
    TrackState<TeKSynth> track1 { 48, false }; // Right Top Track: TeKSynth
    Track3State track2 {};                      // Right Bottom Track: Tekno Tribe Drums
    std::mutex audioMutex;

    std::atomic<bool> isPlaying { false };
    std::atomic<float> bpm { 145.0f };
    std::atomic<int> currentStep { 0 };
    std::atomic<bool> kickPulseTrigger { false };
    std::atomic<bool> synthPulseTrigger { false };
    std::atomic<bool> drumPulseTrigger { false };

    std::atomic<float> sidechainDuckAmount { 0.70f };
    float sidechainEnv = 0.0f;
    float hpBuf = 0.0f;
    float prevSynthSample = 0.0f;

    float sampleRate = 44100.0f;
    uint32_t stepCounter = 0;
    uint32_t samplesPerStep = 0;

    void updateClock()
    {
        // 16th notes: (sampleRate * 60) / (bpm * 4)
        samplesPerStep = static_cast<uint32_t>((sampleRate * 60.0f) / (bpm.load() * 4.0f));
    }
};

extern StudioState studio;

