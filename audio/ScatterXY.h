#pragma once

#include "audio/effects/applyDecimator.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyReverb.h"
#include "audio/effects/applySampleReducer.h"
#include "audio/effects/applyWaveshape.h"
#include <algorithm>
#include <cmath>
#include <cstring>

#define MAX_SCATTER_SAMPLES 192000
#define FX_BUFFER_SIZE 131072

class ScatterXY {
public:
    ScatterXY()
    {
        std::memset(history, 0, sizeof(history));
        std::memset(grain, 0, sizeof(grain));
        std::memset(reverbBuffer, 0, sizeof(reverbBuffer));
    }

    float process(float inputSample, bool active, double samplesPerStep)
    {
        // Continuous history recording
        history[writePtr] = inputSample;
        writePtr = (writePtr + 1) % MAX_SCATTER_SAMPLES;

        if (!active) {
            if (isCaptured) {
                isCaptured = false;
                // Drain reverb tail naturally or clear buffer when finger is lifted
                std::memset(reverbBuffer, 0, sizeof(reverbBuffer));
            }
            return inputSample;
        }

        // Momentary audio snapshot on initial contact
        if (!isCaptured) {
            captureLen = (size_t)(samplesPerStep * 16);
            if (captureLen > MAX_SCATTER_SAMPLES) captureLen = MAX_SCATTER_SAMPLES;
            if (captureLen == 0) captureLen = 1;

            for (size_t i = 0; i < captureLen; i++) {
                size_t hPtr = (writePtr + MAX_SCATTER_SAMPLES - captureLen + i) % MAX_SCATTER_SAMPLES;
                grain[i] = history[hPtr];
            }

            isCaptured = true;
            readPtr = 0.0;
            fDataFx = 0.0;
            iDataFx = 0;
            std::memset(reverbBuffer, 0, sizeof(reverbBuffer));
        }

        if (captureLen == 0) return inputSample;

        // Playhead loop routing
        readPtr = std::fmod(readPtr + 1.0, (double)captureLen);
        size_t i0 = (size_t)readPtr;
        size_t i1 = (i0 + 1) % captureLen;
        float frac = (float)(readPtr - i0);
        float grainSample = grain[i0] + frac * (grain[i1] - grain[i0]);

        // --- VECTOR MORPHING MATH ---
        // Center-relative coordinates ranging from -0.5 to +0.5
        float cx = paramX - 0.5f;
        float cy = paramY - 0.5f;

        // Absolute distances from the dead zone center
        float absX = std::abs(cx) * 2.0f; // Scales out smoothly to 0.0 -> 1.0
        float absY = std::abs(cy) * 2.0f; // Scales out smoothly to 0.0 -> 1.0

        // Initialize target effect levels
        float wWaveshape = 0.0f;
        float wReducer = 0.0f;
        float wDecimator = 0.0f;
        float wReverb = 0.0f;

        // Dead-zone threshold configuration (e.g., 8% center dead zone for pure safety)
        const float DEAD_ZONE = 0.08f;

        // Pure zone threshold configuration (If an axis distance dominates by this much, isolate it)
        const float PURE_ZONE_THRESHOLD = 0.15f;

        if (absX > DEAD_ZONE || absY > DEAD_ZONE) {
            // Determine pure directional focus vs quadrant blend
            if (absX > absY + PURE_ZONE_THRESHOLD) {
                // Pure horizontal strip assignment
                if (cx < 0) wWaveshape = absX;
                else wReducer = absX;
            } else if (absY > absX + PURE_ZONE_THRESHOLD) {
                // Pure vertical strip assignment
                if (cy > 0) wDecimator = absY;
                else wReverb = absY;
            } else {
                // Blended Corner Quadrants: Distribute energy between adjacent parameters
                float totalMag = std::sqrt(absX * absX + absY * absY);
                if (totalMag > 1.0f) totalMag = 1.0f;

                float angle = std::atan2(cy, cx); // Ranging from -PI to +PI

                // Quadrant 1: Top-Right (Reducer + Decimator)
                if (cx >= 0 && cy >= 0) {
                    float blendRatio = angle / (3.14159265f / 2.0f); // 0.0 (Pure Right) to 1.0 (Pure Up)
                    wDecimator = blendRatio * totalMag;
                    wReducer = (1.0f - blendRatio) * totalMag;
                }
                // Quadrant 2: Top-Left (Waveshaper + Decimator)
                else if (cx < 0 && cy >= 0) {
                    float blendRatio = (3.14159265f - angle) / (3.14159265f / 2.0f); // 0.0 (Pure Left) to 1.0 (Pure Up)
                    wDecimator = blendRatio * totalMag;
                    wWaveshape = (1.0f - blendRatio) * totalMag;
                }
                // Quadrant 3: Bottom-Left (Waveshaper + Reverb)
                else if (cx < 0 && cy < 0) {
                    float blendRatio = (-3.14159265f - angle) / (-3.14159265f / 2.0f); // 0.0 (Pure Left) to 1.0 (Pure Down)
                    wReverb = blendRatio * totalMag;
                    wWaveshape = (1.0f - blendRatio) * totalMag;
                }
                // Quadrant 4: Bottom-Right (Reducer + Reverb)
                else if (cx >= 0 && cy < 0) {
                    float blendRatio = angle / (-3.14159265f / 2.0f); // 0.0 (Pure Right) to 1.0 (Pure Down)
                    wReverb = blendRatio * totalMag;
                    wReducer = (1.0f - blendRatio) * totalMag;
                }
            }
        }

        // --- DSP ENGINE STAGE EXECUTION ---
        float outSample = grainSample;

        // 1. Waveshaper Strip (Left side)
        if (wWaveshape > 0.0f) {
            float wet = applyWaveshape2(outSample, wWaveshape);
            outSample = (wet * 0.7f * wWaveshape) + (outSample * (1.0f - (0.7f * wWaveshape)));
        }

        // 2. Sample Reducer Strip (Right side)
        if (wReducer > 0.0f) {
            float factor = 0.1f + ((1.0f - wReducer) * 0.8f); // More intense degradation as weight grows
            outSample = applySampleReducer(outSample, factor, fDataFx, iDataFx);
        }

        // 3. Decimator Strip (Top side)
        if (wDecimator > 0.0f) {
            float factor = 0.1f + ((1.0f - wDecimator) * 0.8f);
            outSample = applyDecimator(outSample, factor, fDataFx, iDataFx);
        }

        // 4. Reverb Strip (Bottom side)
        if (wReverb > 0.0f) {
            outSample = applyReverb(outSample, wReverb, reverbBuffer, reverbIndex);
        }

        return outSample;
    }

    void setXY(float x, float y)
    {
        paramX = std::clamp(x, 0.0f, 1.0f);
        paramY = std::clamp(y, 0.0f, 1.0f);
    }

    float getX() const { return paramX; }
    float getY() const { return paramY; }

private:
    float history[MAX_SCATTER_SAMPLES];
    float grain[MAX_SCATTER_SAMPLES];
    size_t writePtr = 0;
    double readPtr = 0.0;
    size_t captureLen = 0;

    bool isCaptured = false;
    float paramX = 0.5f; // Center default state initialization
    float paramY = 0.5f;

    // Local FX registers
    float fDataFx = 0.0f;
    int iDataFx = 0;

    // Reverb buffer tracking states
    float reverbBuffer[FX_BUFFER_SIZE];
    int reverbIndex = 0;
};