#pragma once

#include "audio/engines/KickWavetable2.h"

#include <alsa/asoundlib.h>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <iostream>

class AudioWorker {
public:
    std::atomic<bool> running { false };

    KickWavetable2 kickEngine;

    // Built-in 64-step kick sequencer state
    std::atomic<bool> playing { false };
    std::atomic<bool> isMuted { false };
    std::atomic<bool> isRepeat { false };

    std::atomic<bool> autoMorphEnabled { false };

    float bpm = 170.0f;
    uint8_t currentStep = 0;

    // Four-on-the-floor kick pattern across 64 steps (bits 0, 4, 8, 12, ... 60)
    uint64_t stepPattern = 0x1111111111111111ULL;

    double samplesPerStep = 0.0;
    double sampleCounter = 0.0;

    AudioWorker(float sampleRate = 44100.0f)
        : kickEngine(sampleRate)
    {
        updateBpm(bpm, sampleRate);
    }

    void updateBpm(float newBpm, float sampleRate = 44100.0f)
    {
        bpm = newBpm;
        // 16th notes: (60 / bpm) / 4 seconds per step
        samplesPerStep = (60.0 / (double)bpm / 4.0) * (double)sampleRate;
    }

    void triggerKick()
    {
        if (!isMuted) {
            kickEngine.trigger(1.0f);
        }
    }

    void startSequencer()
    {
        playing = true;
        currentStep = 0;
        sampleCounter = 0.0;
        if (!isMuted && isStepActive(0)) {
            kickEngine.trigger(1.0f);
        }
    }

    void stopSequencer()
    {
        playing = false;
    }

    void togglePlay()
    {
        if (!playing) {
            startSequencer();
        } else {
            stopSequencer();
        }
    }

    void toggleMute()
    {
        isMuted = !isMuted;
    }

    void toggleRepeat()
    {
        isRepeat = !isRepeat;
    }

    void toggleAutoMorph()
    {
        autoMorphEnabled = !autoMorphEnabled;
    }

    void toggleStep(uint8_t step)
    {
        if (step < 64) {
            stepPattern ^= (1ULL << step);
        }
    }

    bool isStepActive(uint8_t step) const
    {
        if (step >= 64) return false;
        return (stepPattern & (1ULL << step)) != 0;
    }

    void processAudioBlock(float* buffer, int numFrames)
    {
        for (int i = 0; i < numFrames; ++i) {
            if (playing) {
                sampleCounter += 1.0;
                if (sampleCounter >= samplesPerStep) {
                    sampleCounter -= samplesPerStep;
                    currentStep = (currentStep + 1) % 64;

                    if (autoMorphEnabled && (currentStep % 4 == 0)) {
                        float nextVal = kickEngine.waveShape.value + 5.0f;
                        if (nextVal > kickEngine.waveShape.max) nextVal = kickEngine.waveShape.min;
                        kickEngine.waveShape.set(nextVal);
                    }

                    bool shouldTrigger = isStepActive(currentStep);
                    if (isRepeat && (currentStep % 2 == 0)) {
                        shouldTrigger = true;
                    }

                    if (shouldTrigger && !isMuted) {
                        kickEngine.trigger(1.0f);
                    }
                }
            }

            float kOut = kickEngine.sample();

            // Stereo output
            buffer[i * 2 + 0] = kOut;
            buffer[i * 2 + 1] = kOut;
        }
    }
};
