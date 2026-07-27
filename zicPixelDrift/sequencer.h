#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

class Sequencer {
public:
    float bpm = 135.0f;
    int currentStep = 0;
    int totalSteps = 64;

    // Pattern Generator Controls
    float genKick = 0.0f;        // 0.0 (Strict 4-on-the-floor) to 1.0 (Complex Tekno Rolls & Syncopation)
    float synth1TrigMode = 0.0f; // 0: follow, 1: 1, 2: 2, 3: 2-off, 4: 4, 5: 4-off, 6: 8, 7: 16
    float synth2TrigMode = 4.0f; // 0: follow, 1: 1, 2: 2, 3: 2-off, 4: 4, 5: 4-off, 6: 8, 7: 16

    std::vector<std::string> trigDisplayStrings = {
        "follow", "1", "2", "2-off", "4", "4-off", "8", "16"
    };

    bool kickPattern[64] = { false };
    bool isPlaying = true;
    bool isMutatedFill = false;
    bool isKickRepeatActive = false;
    int kickRepeatRate = 2; // Default to 2 (8th notes)

private:
    float sampleRate = 44100.0f;
    double sampleCounter = 0.0;
    double samplesPerStep = 0.0;

    std::mt19937 rng { 12345 };

    float rand01()
    {
        return std::uniform_real_distribution<float>(0.0f, 1.0f)(rng);
    }

    void calculateSamplesPerStep()
    {
        samplesPerStep = (60.0 / bpm / 4.0) * sampleRate;
    }

public:
    Sequencer(float sr = 44100.0f)
        : sampleRate(sr)
    {
        calculateSamplesPerStep();
        updateKickEuclidean();
    }

    double getSamplesPerStep() const
    {
        return samplesPerStep;
    }

    void setSampleRate(float sr)
    {
        sampleRate = sr;
        calculateSamplesPerStep();
    }

    void setBpm(float newBpm)
    {
        bpm = std::clamp(newBpm, 40.0f, 260.0f);
        calculateSamplesPerStep();
    }

    // Exact zicDropV2 Kick Pattern Generator (Extended to 64 steps / 4 bars)
    void updateKickEuclidean()
    {
        for (int i = 0; i < totalSteps; ++i) {
            kickPattern[i] = false;
        }

        // 1. Base 4-on-the-floor kick across all 64 steps
        for (int i = 0; i < totalSteps; i += 4) {
            kickPattern[i] = true;
        }

        float p = std::clamp(genKick, 0.0f, 1.0f);
        if (p <= 0.001f) return;

        rng.seed(12345 + (uint32_t)(p * 100.0f));

        for (int block = 0; block < 4; ++block) {
            int offset = block * 16;

            // 2. Offbeat Bounces (Steps 6, 10 in each 16-step block)
            if (rand01() < (p * 0.70f)) kickPattern[offset + 6] = true;
            if (rand01() < (p * 0.55f)) kickPattern[offset + 10] = true;

            // 3. Phrase-End Rolls (higher probability on block 3 / 4th bar)
            float rollProb = (block == 3) ? (p * 0.85f) : (p * 0.35f);
            if (rand01() < rollProb) {
                kickPattern[offset + 14] = true;
            }
            if (kickPattern[offset + 14] && rand01() < (p * 0.65f)) {
                kickPattern[offset + 15] = true;
            }
            if (p > 0.6f && rand01() < ((p - 0.4f) * 0.50f)) {
                kickPattern[offset + 13] = true;
            }

            // 4. Controlled Syncopated Ghosts (> 50% on knob)
            if (p > 0.5f) {
                float syncopStrength = (p - 0.5f) * 2.0f;
                if (rand01() < (syncopStrength * 0.40f)) {
                    kickPattern[offset + 2] = true;
                }
                if (kickPattern[offset + 6] && rand01() < (syncopStrength * 0.35f)) {
                    kickPattern[offset + 7] = true;
                }
            }
        }
    }

    bool shouldTrigSynth(int trigMode, int step, bool kickTrigged)
    {
        switch (trigMode) {
        case 0: return kickTrigged; // follow
        case 1: return true; // every step
        case 2: return (step % 2 == 0); // every 2 steps
        case 3: return (step % 2 == 1); // 2-off
        case 4: return (step % 4 == 0); // every 4 steps
        case 5: return (step % 4 == 2); // 4-off
        case 6: return (step % 8 == 0); // every 8 steps
        case 7: return (step % 16 == 0); // every 16 steps
        default: return false;
        }
    }

    // Advance clock by 1 audio sample; returns true if a step tick occurs
    bool tick(bool& trigKick, bool& trigSynth1, bool& trigSynth2, float& velocity)
    {
        if (!isPlaying) return false;

        sampleCounter += 1.0;
        if (sampleCounter >= samplesPerStep) {
            sampleCounter -= samplesPerStep;
            currentStep = (currentStep + 1) % totalSteps;

            if (isKickRepeatActive) {
                trigKick = (currentStep % kickRepeatRate == 0);
            } else {
                trigKick = kickPattern[currentStep] || (isMutatedFill && (currentStep % 2 == 0));
            }
            velocity = 1.0f;

            trigSynth1 = shouldTrigSynth((int)std::round(synth1TrigMode), currentStep, trigKick);
            trigSynth2 = shouldTrigSynth((int)std::round(synth2TrigMode), currentStep, trigKick);

            return true;
        }
        return false;
    }
};
