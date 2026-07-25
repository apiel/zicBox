#pragma once

#include <algorithm>
#include <cmath>

class Sequencer {
public:
    float bpm = 135.0f;
    int currentStep = 0;
    int totalSteps = 16;

    // Euclidean & Pattern Controls
    int euclidPulses = 4;        // 1 to 16 pulses
    float gateLength = 0.5f;     // 0.1 to 1.0
    float mutationRate = 0.0f;   // 0.0 to 1.0 (Probability of extra ghost pulses)

    bool kickPattern[16] = {
        true, false, false, false,
        true, false, false, false,
        true, false, false, false,
        true, false, false, false
    };

    bool activeEuclid[16] = { false };
    bool isPlaying = true;
    bool isMutatedFill = false;

private:
    float sampleRate = 44100.0f;
    double sampleCounter = 0.0;
    double samplesPerStep = 0.0;

    void calculateSamplesPerStep()
    {
        // 16th notes: (60 / bpm) / 4 * sampleRate
        samplesPerStep = (60.0 / bpm / 4.0) * sampleRate;
    }

public:
    Sequencer(float sr = 44100.0f)
        : sampleRate(sr)
    {
        calculateSamplesPerStep();
        updateEuclideanPattern();
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

    void updateEuclideanPattern()
    {
        int k = std::clamp(euclidPulses, 1, totalSteps);
        int n = totalSteps;

        for (int i = 0; i < n; ++i) {
            activeEuclid[i] = ((i * k) % n) < k;
        }
    }

    // Advance clock by 1 audio sample; returns true if a step trigger occurs
    bool tick(bool& isKickTrigger, float& velocity)
    {
        if (!isPlaying) return false;

        sampleCounter += 1.0;
        if (sampleCounter >= samplesPerStep) {
            sampleCounter -= samplesPerStep;
            currentStep = (currentStep + 1) % totalSteps;

            isKickTrigger = kickPattern[currentStep] || (isMutatedFill && (currentStep % 2 == 0));
            velocity = 1.0f;

            if (isKickTrigger && currentStep % 4 != 0) {
                // Ghost accent / syncopation
                velocity = 0.75f;
            }

            return true;
        }
        return false;
    }
};
