#pragma once

#include "audio/engines/TeKKick.h"
#include "audio/engines/TeKSynth.h"
#include "audio/sequencer/Step.h"
#include <atomic>
#include <mutex>
#include <vector>

#ifndef MAX_TRACKS
#define MAX_TRACKS 2
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

struct StudioState {
    TrackState<TeKKick> track0 { 60, true };   // Left Track: Massive Kick
    TrackState<TeKSynth> track1 { 48, false }; // Right Track: TeKSynth
    std::mutex audioMutex;

    std::atomic<bool> isPlaying { false };
    std::atomic<float> bpm { 145.0f };
    std::atomic<int> currentStep { 0 };
    std::atomic<bool> kickPulseTrigger { false };
    std::atomic<bool> synthPulseTrigger { false };

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
