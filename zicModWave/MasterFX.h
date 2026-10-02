#pragma once

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

class MasterFX {
public:
    static constexpr int DELAY_BUF_SIZE = 8192;
    float* delayBuffer = nullptr;
    int delayWriteIdx = 0;
    float delayTimeMs = 180.0f;
    float delayFeedback = 0.50f;
    float masterVol = 80.0f; // 0..100%

    MasterFX()
    {
        delayBuffer = (float*)calloc(DELAY_BUF_SIZE, sizeof(float));
    }

    ~MasterFX()
    {
        if (delayBuffer) {
            free(delayBuffer);
            delayBuffer = nullptr;
        }
    }

    float process(float drySample, float delaySendPct)
    {
        if (!delayBuffer) return drySample * (masterVol * 0.01f);

        float sendGain = std::clamp(delaySendPct * 0.01f, 0.0f, 1.0f);

        // Calculate sample delay offset
        int delaySamples = (int)(delayTimeMs * 0.001f * 44100.0f);
        delaySamples = std::clamp(delaySamples, 100, DELAY_BUF_SIZE - 1);

        int readIdx = (delayWriteIdx - delaySamples + DELAY_BUF_SIZE) % DELAY_BUF_SIZE;
        float wetSample = delayBuffer[readIdx];

        // Write into delay line with feedback
        delayBuffer[delayWriteIdx] = drySample * sendGain + wetSample * delayFeedback;
        delayWriteIdx = (delayWriteIdx + 1) % DELAY_BUF_SIZE;

        float mixedSample = drySample + wetSample * sendGain;
        return mixedSample * (masterVol * 0.01f);
    }
};
