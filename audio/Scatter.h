#pragma once

#include "audio/effects/applyDecimator.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applySampleReducer.h"
#include "audio/effects/applyWaveshape.h"
#include "audio/effects/applyReverb.h"
#include "audio/filterArray.h"
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
        std::memset(modeMix, 0, sizeof(modeMix));
    }

    void toggleMode(int mode)
    {
        if (mode >= 0 && mode <= 3) {
            activeModes[mode] = !activeModes[mode];
        }
    }

    bool anyActive() const
    {
        return activeModes[0] || activeModes[1] || activeModes[2] || activeModes[3];
    }

    bool anyActiveOrReleasing() const
    {
        return anyActive() || modeMix[0] > 0.0f || modeMix[1] > 0.0f || modeMix[2] > 0.0f || modeMix[3] > 0.0f;
    }

    float process(float input, double samplesPerStep)
    {
        hist[writePtr] = input;
        writePtr = (writePtr + 1) % MAX_SCATTER_SAMPLES;

        double decay = (samplesPerStep > 0.0) ? (1.0 / (samplesPerStep * 1.5)) : 0.01;
        for (int i = 0; i < 4; i++) {
            if (activeModes[i]) {
                modeMix[i] = 1.0f;
            } else if (modeMix[i] > 0.0f) {
                modeMix[i] -= decay;
                if (modeMix[i] < 0.0f) {
                    modeMix[i] = 0.0f;
                }
            }
        }

        if (!anyActiveOrReleasing()) {
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

        float out = input;
        if (modeMix[0] > 0.0f) {
            lfoPhase += 1.0 / (samplesPerStep > 0.0 ? samplesPerStep * 4.0 : 40000.0);
            if (lfoPhase >= 1.0) lfoPhase -= 1.0;
            double lfoVal = sin(lfoPhase * 2.0 * 3.14159265358979323846);
            double delaySamples = 50.0 + 350.0 * (0.5 + 0.5 * lfoVal);
            float delayedSample = readHistAtDelay(delaySamples);
            float feedback = 0.8f;
            float combOut = input + feedback * delayedSample;

            float cutoff = 0.15f + 0.3f * (float)(0.5 + 0.5 * lfoVal);
            filter.setCutoff(cutoff);
            filter.setResonance(0.4f);
            filter.setSampleData(combOut, 0);

            float fxOut = filter.lp[0] * 0.7f + filter.bp[0] * 0.3f;
            out = modeMix[0] * fxOut + (1.0f - modeMix[0]) * out;
        }

        if (modeMix[1] > 0.0f) {
            float fxOut = applySampleReducer(out, 0.1, fDataFx, iDataFx);
            out = modeMix[1] * fxOut + (1.0f - modeMix[1]) * out;
        }
        if (modeMix[2] > 0.0f) {
            float fxOut = applyDecimator(out, 0.5, fDataFx, iDataFx);
            out = modeMix[2] * fxOut + (1.0f - modeMix[2]) * out;
        }
        if (modeMix[3] > 0.0f) {
            float fxOut = applyReverb(out, 0.7f, reverbBuffer, reverbIndex);
            out = modeMix[3] * fxOut + (1.0f - modeMix[3]) * out;
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
    float modeMix[4];
    float reverbBuffer[FX_BUFFER_SIZE];
    int reverbIndex = 0;
    EffectFilterArray<1> filter;
    double lfoPhase = 0.0;

    float readHistAtDelay(double delaySamples)
    {
        double ptr = (double)writePtr - delaySamples;
        while (ptr < 0.0) ptr += MAX_SCATTER_SAMPLES;
        while (ptr >= MAX_SCATTER_SAMPLES) ptr -= MAX_SCATTER_SAMPLES;
        size_t i0 = (size_t)ptr;
        size_t i1 = (i0 + 1) % MAX_SCATTER_SAMPLES;
        float frac = (float)(ptr - i0);
        return hist[i0] + frac * (hist[i1] - hist[i0]);
    }

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