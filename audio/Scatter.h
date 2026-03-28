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
        std::memset(histDrums, 0, sizeof(histDrums));
        std::memset(histSynth, 0, sizeof(histSynth));
        std::memset(grainDrums, 0, sizeof(grainDrums));
        std::memset(grainSynth, 0, sizeof(grainSynth));
    }

    float process(float drumIn, float synthIn, int mode, double samplesPerStep)
    {
        histDrums[writePtr] = drumIn;
        histSynth[writePtr] = synthIn;
        writePtr = (writePtr + 1) % MAX_SCATTER_SAMPLES;

        if (mode == 0) {
            isCaptured = false;
            return drumIn + synthIn;
        }

        if (!isCaptured) {
            captureLen = (size_t)(samplesPerStep * 16);
            if (captureLen > MAX_SCATTER_SAMPLES) captureLen = MAX_SCATTER_SAMPLES;

            for (size_t i = 0; i < captureLen; i++) {
                size_t hPtr = (writePtr + MAX_SCATTER_SAMPLES - captureLen + i) % MAX_SCATTER_SAMPLES;
                grainDrums[i] = histDrums[hPtr];
                grainSynth[i] = histSynth[hPtr];
            }
            isCaptured = true;
            readPtrDrums = 0.0;
            readPtrSynth = 0.0;
            case5Timer = 0;
        }

        return apply12Modes(mode, samplesPerStep);
    }

private:
    float histDrums[MAX_SCATTER_SAMPLES], histSynth[MAX_SCATTER_SAMPLES];
    float grainDrums[MAX_SCATTER_SAMPLES], grainSynth[MAX_SCATTER_SAMPLES];
    size_t writePtr = 0;
    double readPtrDrums = 0.0, readPtrSynth = 0.0;
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
    float apply12Modes(int mode, double sPS)
    {
        float outD = 0, outS = 0;
        double speedD = 1.0, speedS = 1.0;

        switch (mode) {
        case 1: // 1-Step Retrig
            readPtrDrums = fmod(readPtrDrums + 1.0, sPS);
            readPtrSynth = fmod(readPtrSynth + 1.0, (double)captureLen);
            break;

        case 2: // 2-Step Retrig
            readPtrDrums = fmod(readPtrDrums + 1.0, sPS * 2.0);
            readPtrSynth = fmod(readPtrSynth + 1.0, (double)captureLen);
            break;

        case 3: {
            outD = readBuffer(grainDrums, readPtrDrums = fmod(readPtrDrums + 1.0, (double)captureLen));
            outD = applyWaveshape2(outD, 0.5);
            outD = outD * 0.7f + grainDrums[(size_t)readPtrDrums] * 0.3f;
            readPtrSynth = fmod(readPtrSynth + 1.0, (double)captureLen);
            return outD + readBuffer(grainSynth, readPtrSynth);
        }
        case 4: {
            outD = readBuffer(grainDrums, readPtrDrums = fmod(readPtrDrums + 1.0, (double)captureLen));
            outD = applySampleReducer(outD, 0.3, fDataFx, iDataFx);
            readPtrSynth = fmod(readPtrSynth + 1.0, (double)captureLen);
            return outD + readBuffer(grainSynth, readPtrSynth);
        }
        case 5:
            outD = readBuffer(grainDrums, readPtrDrums = fmod(readPtrDrums + 1.0, (double)captureLen));
            outD = applyDecimator(outD, 0.5, fDataFx, iDataFx);
            readPtrSynth = fmod(readPtrSynth + 1.0, (double)captureLen);
            return outD + readBuffer(grainSynth, readPtrSynth);
            break;
        case 6:
            speedS = 0.75;
            readPtrDrums = fmod(readPtrDrums + speedD, (double)captureLen);
            readPtrSynth = fmod(readPtrSynth + speedS, (double)captureLen);
            break;

        case 7:
            speedS = 1.5;
            readPtrDrums = fmod(readPtrDrums + speedD, (double)captureLen);
            readPtrSynth = fmod(readPtrSynth + speedS, (double)captureLen);
            break;

        case 8: {
            // readPtrDrums = (double)captureLen - fmod(readPtrSynth + 1.0, (double)captureLen);
            // readPtrSynth = fmod(readPtrSynth + 1.0, (double)captureLen);
            // break;

            readPtrDrums = fmod(readPtrDrums + 1.0, (double)captureLen);
            readPtrSynth = fmod(readPtrSynth + 1.0, (double)captureLen);
            double revD = (double)(captureLen - 1) - readPtrDrums;
            double revS = (double)(captureLen - 1) - readPtrSynth;
            return readBuffer(grainDrums, revD) + readBuffer(grainSynth, revS);
        }
        case 9:
            readPtrDrums = fmod(readPtrDrums + 1.0, (double)captureLen);
            readPtrSynth = fmod(readPtrSynth + 1.0, 200.0);
            break;

        case 10:
            readPtrDrums = fmod(readPtrDrums + 1.0, (double)captureLen);
            readPtrSynth = fmod(readPtrSynth + 1.0, 100.0);
            break;

        case 11:
            speedS = 1.0 + (0.2 * sin(readPtrSynth * 0.05));
            readPtrDrums = fmod(readPtrDrums + 1.0, (double)captureLen);
            readPtrSynth = fmod(readPtrSynth + speedS, (double)captureLen);
            break;

        default:
            readPtrDrums = fmod(readPtrDrums + 1.0, (double)captureLen);
            readPtrSynth = fmod(readPtrSynth + 1.0, (double)captureLen);
            break;
        }

        outD = readBuffer(grainDrums, readPtrDrums);
        outS = readBuffer(grainSynth, readPtrSynth);

        return outD + outS;
    }
};