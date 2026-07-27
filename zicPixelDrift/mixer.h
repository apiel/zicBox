#pragma once

#include <algorithm>
#include <cmath>

class Mixer {
public:
    // Master Page 1: Volume, Mix & Shared Delay
    float volume = 0.70f; // 0.0 to 1.0 (Drive active when > 0.60)
    float mix = 0.50f; // 0.0 (Kick only) to 1.0 (Synths only)
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

    float process(float kickSample, float synth1Sample, float synth1DelaySend, float synth2Sample, float synth2DelaySend)
    {
        // 1. Process Shared Master Delay Line for Synths
        float delaySendSum = (synth1Sample * (synth1DelaySend * 0.01f)) + (synth2Sample * (synth2DelaySend * 0.01f));
        int delaySamples = std::clamp((int)(delayTimeMs * 0.001f * sampleRate), 1, DELAY_BUF_SIZE - 1);
        int readPos = (delayWrite - delaySamples + DELAY_BUF_SIZE) % DELAY_BUF_SIZE;
        float delayOut = delayBuffer[readPos];

        delayBuffer[delayWrite] = delaySendSum + (delayOut * delayFeedback);
        delayWrite = (delayWrite + 1) % DELAY_BUF_SIZE;

        // 2. Mix Synths + Delay
        float totalSynths = (synth1Sample * 0.3f) + (synth2Sample * 0.3f) + (delayOut * 0.8f);

        float summed = ((kickSample * 0.3f) * (1.0f - mix)) + (totalSynths * mix);

        // 4. Master Volume Dual Function (0.0 to 0.60 clean, >0.60 adds Overdrive Saturation)
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


