#pragma once

#include "audio/effects/applyDecimator.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applySampleReducer.h"
#include "audio/effects/applyWaveshape.h"
#include <algorithm>
#include <cmath>
#include <cstring>

#define MAX_SCATTER_SAMPLES 192000

class ScatterXY {
public:
    ScatterXY()
    {
        std::memset(history, 0, sizeof(history));
        std::memset(grain, 0, sizeof(grain));
    }

    // Pass the active state and timing metrics directly inline with the audio stream
    float process(float inputSample, bool active, double samplesPerStep)
    {
        // Continuous recording into the background loop history array
        history[writePtr] = inputSample;
        writePtr = (writePtr + 1) % MAX_SCATTER_SAMPLES;

        // When the user lifts their finger off the XY pad
        if (!active) {
            isCaptured = false;
            return inputSample; // Clean bypass passthrough
        }

        // Catch the initial state transition (The exact sample the pad is touched)
        if (!isCaptured) {
            captureLen = (size_t)(samplesPerStep * 16);
            if (captureLen > MAX_SCATTER_SAMPLES) captureLen = MAX_SCATTER_SAMPLES;
            if (captureLen == 0) captureLen = 1; // Div by zero fallback

            // Snap the recent rolling history into our grain play buffer
            for (size_t i = 0; i < captureLen; i++) {
                size_t hPtr = (writePtr + MAX_SCATTER_SAMPLES - captureLen + i) % MAX_SCATTER_SAMPLES;
                grain[i] = history[hPtr];
            }
            
            isCaptured = true;
            readPtr = 0.0;
            fDataFx = 0.0;
            iDataFx = 0;
        }

        // Safe loop fallback block
        if (captureLen == 0) return inputSample;

        // Advance buffer position pointer (Looping the audio slice)
        readPtr = std::fmod(readPtr + 1.0, (double)captureLen);

        // Linear sample interpolation lookup
        size_t i0 = (size_t)readPtr;
        size_t i1 = (i0 + 1) % captureLen;
        float frac = (float)(readPtr - i0);
        float outSample = grain[i0] + frac * (grain[i1] - grain[i0]);

        // Route DSP modifications based on active UI coordinates
        if (paramX < 0.33f) {
            // TYPE 1: Waveshaper
            float intensity = paramY; 
            float wet = applyWaveshape2(outSample, intensity);
            outSample = (wet * 0.7f) + (outSample * 0.3f);
        } 
        else if (paramX >= 0.33f && paramX < 0.66f) {
            // TYPE 2: Sample Reducer
            float factor = 0.1f + (paramY * 0.8f); 
            outSample = applySampleReducer(outSample, factor, fDataFx, iDataFx);
        } 
        else {
            // TYPE 3: Decimator
            float factor = 0.1f + (paramY * 0.8f);
            outSample = applyDecimator(outSample, factor, fDataFx, iDataFx);
        }

        return outSample;
    }

    // X/Y coordinates fed seamlessly from interface events
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
    float paramX = 0.0f;
    float paramY = 0.0f;

    // Local registers for effect instances
    float fDataFx = 0.0f;
    int iDataFx = 0;
};