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
        float cx = paramX - 0.5f;
        float cy = paramY - 0.5f;

        float absX = std::abs(cx) * 2.0f;
        float absY = std::abs(cy) * 2.0f;

        float wWaveshape = 0.0f;
        float wReducer = 0.0f;
        float wDecimator = 0.0f;
        float wReverb = 0.0f;

        const float DEAD_ZONE = 0.08f;
        const float PURE_ZONE_THRESHOLD = 0.15f;

        if (absX > DEAD_ZONE || absY > DEAD_ZONE) {
            if (absX > absY + PURE_ZONE_THRESHOLD) {
                if (cx < 0) wWaveshape = absX;
                else wReducer = absX;
            } else if (absY > absX + PURE_ZONE_THRESHOLD) {
                if (cy > 0) wDecimator = absY;
                else wReverb = absY;
            } else {
                float totalMag = std::sqrt(absX * absX + absY * absY);
                if (totalMag > 1.0f) totalMag = 1.0f;

                float angle = std::atan2(cy, cx);

                // Quadrant 1: Top-Right (Reducer + Decimator)
                if (cx >= 0 && cy >= 0) {
                    float blendRatio = angle / (1.57079632f);
                    wDecimator = blendRatio * totalMag;
                    wReducer = (1.0f - blendRatio) * totalMag;
                }
                // Quadrant 2: Top-Left (Waveshaper + Decimator)
                else if (cx < 0 && cy >= 0) {
                    float blendRatio = (3.14159265f - angle) / (1.57079632f);
                    wDecimator = blendRatio * totalMag;
                    wWaveshape = (1.0f - blendRatio) * totalMag;
                }
                // Quadrant 3: Bottom-Left (Waveshaper + Reverb)
                else if (cx < 0 && cy < 0) {
                    float blendRatio = (-3.14159265f - angle) / (-1.57079632f);
                    wReverb = blendRatio * totalMag;
                    wWaveshape = (1.0f - blendRatio) * totalMag;
                }
                // Quadrant 4: Bottom-Right (Reducer + Reverb)
                else if (cx >= 0 && cy < 0) {
                    float blendRatio = angle / (-1.57079632f);
                    wReverb = blendRatio * totalMag;
                    wReducer = (1.0f - blendRatio) * totalMag;
                }
            }
        }

        // --- PARALLEL SMOOTH DSP EXECUTION ---
        // Instead of daisy-chaining effects which compound artifacts,
        // we process each active component cleanly out of the grain sample and sum via mix weights.

        float accumulatedSample = 0.0f;
        float totalActiveWeight = 0.0f;

        // 1. Waveshaper
        if (wWaveshape > 0.0f) {
            float wet = applyWaveshape2(grainSample, wWaveshape);
            // Blend wet and dry for this specific effect slot
            float slotSample = (wet * 0.7f) + (grainSample * 0.3f);
            accumulatedSample += slotSample * wWaveshape;
            totalActiveWeight += wWaveshape;
        }

        // 2. Sample Reducer
        if (wReducer > 0.0f) {
            // Fix intensity: mapping it so max weight = max downsampling smoothly
            float factor = 0.9f - (wReducer * 0.7f); // Moves cleanly from 0.9 (smooth) to 0.2 (crunchy)
            float slotSample = applySampleReducer(grainSample, factor, fDataFx, iDataFx);
            accumulatedSample += slotSample * wReducer;
            totalActiveWeight += wReducer;
        }

        // 3. Decimator
        if (wDecimator > 0.0f) {
            float factor = 0.9f - (wDecimator * 0.7f);
            float slotSample = applyDecimator(grainSample, factor, fDataFx, iDataFx);
            accumulatedSample += slotSample * wDecimator;
            totalActiveWeight += wDecimator;
        }

        // 4. Reverb
        if (wReverb > 0.0f) {
            float slotSample = applyReverb(grainSample, wReverb, reverbBuffer, reverbIndex);
            accumulatedSample += slotSample * wReverb;
            totalActiveWeight += wReverb;
        }

        // Final Crossfade mix: Blend accumulated processed signals with pristine grain loop
        if (totalActiveWeight > 0.0f) {
            if (totalActiveWeight > 1.0f) totalActiveWeight = 1.0f;
            float processedMix = accumulatedSample / totalActiveWeight;
            return (processedMix * totalActiveWeight) + (grainSample * (1.0f - totalActiveWeight));
        }

        return grainSample;
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