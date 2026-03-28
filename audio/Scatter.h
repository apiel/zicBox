#pragma once

#include <algorithm>
#include <cmath>
#include <cstring>

#define MAX_SCATTER_SAMPLES 192000 // ~4 seconds at 48k

class Scatter {
public:
    Scatter()
    {
        std::memset(history, 0, sizeof(history));
        std::memset(activeGrain, 0, sizeof(activeGrain));
    }

    // Call this in your audio loop
    float process(float input, int mode, double samplesPerStep, float stepProgress)
    {
        // 1. Constant Recording into History
        history[writePtr] = input;
        writePtr = (writePtr + 1) % MAX_SCATTER_SAMPLES;

        if (mode == 0) {
            isCaptured = false;
            return input;
        }

        // 2. Capture on Trigger
        if (!isCaptured) {
            captureLen = (size_t)(samplesPerStep * 4); // Capture 1 bar (4 steps)
            if (captureLen > MAX_SCATTER_SAMPLES) captureLen = MAX_SCATTER_SAMPLES;

            // Copy from history backwards into activeGrain
            for (size_t i = 0; i < captureLen; i++) {
                size_t hPtr = (writePtr + MAX_SCATTER_SAMPLES - captureLen + i) % MAX_SCATTER_SAMPLES;
                activeGrain[i] = history[hPtr];
            }
            isCaptured = true;
            readPtr = 0.0;
        }

        // 3. Playback Logic (The "Scatter" Performance)
        return applyScatterLogic(mode, samplesPerStep, stepProgress);
    }

private:
    float history[MAX_SCATTER_SAMPLES];
    float activeGrain[MAX_SCATTER_SAMPLES];
    size_t writePtr = 0;
    double readPtr = 0.0;
    size_t captureLen = 0;
    bool isCaptured = false;

    float applyScatterLogic(int mode, double samplesPerStep, float progress)
    {
        float speed = 1.0f;
        float finalSample = 0.0f;

        switch (mode) {
        case 1: // Classic 1/16th Retrig
            // Loop only the first 1/4th of our captured bar (1 step)
            readPtr = fmod(readPtr + 1.0, samplesPerStep);
            break;

        case 2: // Double Speed (Pitch Up Octave)
            speed = 2.0f;
            readPtr = fmod(readPtr + speed, (double)captureLen);
            break;

        case 3: // Chromatic / Granular Pitch
            // To do this smoothly, you'd use a small windowed grain
            speed = 0.5f; // Half speed / Octave down
            readPtr = fmod(readPtr + speed, (double)captureLen);
            break;

        case 4: // Reverse Bar
            readPtr = (double)captureLen - fmod(readPtr + 1.0, (double)captureLen);
            if (readPtr < 0) readPtr = 0;
            break;

        default:
            readPtr = fmod(readPtr + 1.0, (double)captureLen);
            break;
        }

        // Linear Interpolation for smooth pitch shifting
        size_t i0 = (size_t)readPtr;
        size_t i1 = (i0 + 1) % captureLen;
        float frac = (float)(readPtr - i0);

        finalSample = activeGrain[i0] + frac * (activeGrain[i1] - activeGrain[i0]);

        // Simple Low-Pass Filter "Built-in" to the effect
        // (You could add your trk->eq.process logic here)

        return finalSample;
    }
};