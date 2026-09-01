#pragma once

#include <algorithm>
#include <atomic>
#include <cmath>

class Mixer {
public:
    // Master Page 1: Volume
    float volume = 0.85f; // 0.0 to 1.0 (Drive active when > 0.60)

    // Master Page 2: Shared Delay
    float delayTimeMs = 250.0f; // 10 to 1000 ms
    float delayFeedback = 0.35f; // 0.0 to 0.95

private:
    static const int DELAY_BUF_SIZE = 48000;
    float delayBuffer[DELAY_BUF_SIZE] = { 0.0f };
    int delayWrite = 0;
    float sampleRate = 44100.0f;

public:
    Mixer(float sr = 44100.0f)
        : sampleRate(sr)
    {
    }

    float process(float drumSample, float synth1Sample, float synth1DelaySend, float synth2Sample, float synth2DelaySend)
    {
        float d = drumSample;
        float s1 = synth1Sample;
        float s2 = synth2Sample;

        // 1. Process Shared Master Delay Line
        float delaySendSum = (s1 * (synth1DelaySend * 0.01f)) + (s2 * (synth2DelaySend * 0.01f)) + (d * 0.15f);
        int delaySamples = std::clamp((int)(delayTimeMs * 0.001f * sampleRate), 1, DELAY_BUF_SIZE - 1);
        int readPos = (delayWrite - delaySamples + DELAY_BUF_SIZE) % DELAY_BUF_SIZE;
        float delayOut = delayBuffer[readPos];

        delayBuffer[delayWrite] = delaySendSum + (delayOut * delayFeedback);
        delayWrite = (delayWrite + 1) % DELAY_BUF_SIZE;

        float summed = (d + s1 + s2 + delayOut);

        // 2. Master Volume Dual Function (0.0 to 0.60 clean, >0.60 adds Overdrive Saturation)
        float output = 0.0f;
        if (volume <= 0.60f) {
            float gain = volume / 0.60f;
            output = summed * gain;
        } else {
            float overdriveAmt = (volume - 0.60f) / 0.40f;
            float gain = 1.0f + (overdriveAmt * 3.0f);
            float driven = std::tanh(summed * gain);
            output = driven;
        }

        return std::clamp(output, -1.0f, 1.0f);
    }
};
