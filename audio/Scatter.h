#pragma once

#include "audio/effects/applyDecimator.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applySampleReducer.h"
#include "audio/effects/applyWaveshape.h"
#include "audio/effects/applyReverb.h"
#include "helpers/clamp.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdint>

#define MAX_SCATTER_SAMPLES 192000
#define FX_BUFFER_SIZE 131072

class Scatter {
public:
    Scatter()
    {
        std::memset(hist, 0, sizeof(hist));
        std::memset(grain, 0, sizeof(grain));
        std::memset(reverbBuffer, 0, sizeof(reverbBuffer));
        std::memset(activeModes, 0, sizeof(activeModes));
    }

    void toggleMode(int mode)
    {
        if (mode >= 0 && mode < 8) {
            activeModes[mode] = !activeModes[mode];
        }
    }

    bool anyActive() const
    {
        return activeModes[4] || activeModes[5] || activeModes[6] || activeModes[7];
    }

    float process(float input, double samplesPerStep)
    {
        hist[writePtr] = input;
        writePtr = (writePtr + 1) % MAX_SCATTER_SAMPLES;

        if (!anyActive()) {
            isCaptured = false;
            return input;
        }

        if (!isCaptured) {
            captureLen = (size_t)(samplesPerStep * 16);
            if (captureLen > MAX_SCATTER_SAMPLES) captureLen = MAX_SCATTER_SAMPLES;

            for (size_t i = 0; i < captureLen; i++) {
                size_t hPtr = (writePtr + MAX_SCATTER_SAMPLES - captureLen + i) % MAX_SCATTER_SAMPLES;
                grain[i] = hist[hPtr];
            }
            isCaptured = true;
            readPtr = 0.0;
            case5Timer = 0;
            std::memset(reverbBuffer, 0, sizeof(reverbBuffer));
        }

        if (activeModes[7]) {
            readPtr = fmod(readPtr + 1.0, samplesPerStep * 4.0);
        } else {
            readPtr = fmod(readPtr + 1.0, (double)captureLen);
        }

        float out = readBuffer(grain, readPtr);

        if (activeModes[4]) {
            out = applySampleReducer(out, 0.3, fDataFx, iDataFx);
        }
        if (activeModes[5]) {
            out = applyDecimator(out, 0.5, fDataFx, iDataFx);
        }
        if (activeModes[6]) {
            out = applyReverb(out, 0.7f, reverbBuffer, reverbIndex);
        }

        return out;
    }

private:
    float hist[MAX_SCATTER_SAMPLES];
    float grain[MAX_SCATTER_SAMPLES];
    size_t writePtr = 0;
    double readPtr = 0.0;
    size_t captureLen = 0;
    bool isCaptured = false;
    uint32_t case5Timer = 0;

    bool activeModes[8];
    float reverbBuffer[FX_BUFFER_SIZE];
    int reverbIndex = 0;

    float readBuffer(float* buf, double& ptr)
    {
        size_t i0 = (size_t)ptr;
        size_t i1 = (i0 + 1) % captureLen;
        float frac = (float)(ptr - i0);
        return buf[i0] + frac * (buf[i1] - buf[i0]);
    }

    float fDataFx = 0.0;
    int iDataFx = 0;
};