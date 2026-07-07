#pragma once

#include "audio/effects/applyDecimator.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applySampleReducer.h"
#include "audio/effects/applyWaveshape.h"
#include "helpers/clamp.h"
#include <algorithm>
#include <cmath>
#include <cstring>

#define MAX_SCATTER_SAMPLES 192000

class Scatter {
public:
    Scatter()
    {
        std::memset(hist, 0, sizeof(hist));
        std::memset(grain, 0, sizeof(grain));
    }

    float process(float input, int mode, double samplesPerStep)
    {
        hist[writePtr] = input;
        writePtr = (writePtr + 1) % MAX_SCATTER_SAMPLES;

        if (mode == 0) {
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
        }

        return apply(mode, samplesPerStep);
    }

private:
    float hist[MAX_SCATTER_SAMPLES];
    float grain[MAX_SCATTER_SAMPLES];
    size_t writePtr = 0;
    double readPtr = 0.0;
    size_t captureLen = 0;
    bool isCaptured = false;
    uint32_t case5Timer = 0;

    float readBuffer(float* buf, double& ptr)
    {
        size_t i0 = (size_t)ptr;
        size_t i1 = (i0 + 1) % captureLen;
        float frac = (float)(ptr - i0);
        return buf[i0] + frac * (buf[i1] - buf[i0]);
    }

    float fDataFx = 0.0;
    int iDataFx = 0;
    float apply(int mode, double sPS)
    {
        float outD = 0, outS = 0;
        double speedD = 1.0, speedS = 1.0;

        switch (mode) {
        case 1: // 1-Step Retrig
            readPtr = fmod(readPtr + 1.0, sPS);
            break;

        case 2: // 2-Step Retrig
            readPtr = fmod(readPtr + 1.0, sPS * 2.0);
            break;

        case 3: {
            outD = readBuffer(grain, readPtr = fmod(readPtr + 1.0, (double)captureLen));
            outD = applyWaveshape2(outD, 0.5);
            outD = outD * 0.7f + grain[(size_t)readPtr] * 0.3f;
            return outD;
        }
        case 4: {
            outD = readBuffer(grain, readPtr = fmod(readPtr + 1.0, (double)captureLen));
            outD = applySampleReducer(outD, 0.3, fDataFx, iDataFx);
            return outD;
        }
        case 5:
            outD = readBuffer(grain, readPtr = fmod(readPtr + 1.0, (double)captureLen));
            outD = applyDecimator(outD, 0.5, fDataFx, iDataFx);
            return outD;
            break;
        case 6:
            speedS = 0.75;
            readPtr = fmod(readPtr + speedD, (double)captureLen);
            break;

        // case 7:
        //     speedS = 1.5;
        //     readPtr = fmod(readPtr + speedD, (double)captureLen);
        //     break;

        // case 8: {
        //     readPtr = fmod(readPtr + 1.0, (double)captureLen);
        //     double revD = (double)(captureLen - 1) - readPtr;
        //     return readBuffer(grain, revD);
        // }
        // case 9:
        //     readPtr = fmod(readPtr + 1.0, (double)captureLen);
        //     break;

        default:
            readPtr = fmod(readPtr + 1.0, (double)captureLen);
            break;
        }

        outD = readBuffer(grain, readPtr);

        return outD;
    }
};