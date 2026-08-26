#pragma once

#include "audio/engines/TeKKick.h"
#include "audio/sequencer/Step.h"
#include <atomic>
#include <mutex>
#include <vector>

#ifndef MAX_TRACKS
#define MAX_TRACKS 4
#endif

#ifndef SEQ_STEPS_TEK
#define SEQ_STEPS_TEK 16
#endif

struct TrackState {
    TeKKick kick;
    Step sequence[SEQ_STEPS_TEK];
    bool isMuted = false;
    float volume = 0.85f;

    TrackState()
    {
        for (int i = 0; i < SEQ_STEPS_TEK; i++) {
            sequence[i].active = (i % 4 == 0); // Default 4-on-the-floor kick pattern
            sequence[i].note = 60;              // C4 default
            sequence[i].velocity = 0.9f;
        }
    }
};

struct StudioState {
    TrackState track0;
    std::mutex audioMutex;

    std::atomic<bool> isPlaying { false };
    std::atomic<float> bpm { 145.0f };
    std::atomic<int> currentStep { 0 };
    std::atomic<bool> kickPulseTrigger { false };

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
