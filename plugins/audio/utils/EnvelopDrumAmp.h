#pragma once
#include <algorithm>
#include <cmath>

class EnvelopDrumAmp {
public:
    EnvelopDrumAmp()
        : sampleCount(1)
        , currentSample(0)
        , morphValue(0.5f)
    {
    }

    // Set morph in [0.0, 1.0]
    void morph(float value)
    {
        morphValue = std::clamp(value, 0.0f, 1.0f);
    }

    // Reset envelope with duration in samples
    void reset(int durationSamples)
    {
        sampleCount = std::max(1, durationSamples);
        currentSample = 0;
    }

    // Returns envelope value in [0.0f, 1.0f]
    float next()
    {
        if (currentSample >= sampleCount)
            return 0.0f;

        float phase = static_cast<float>(currentSample++) / sampleCount;
        return applyMorphShape(1.0f - phase);
    }

private:
    int sampleCount;
    int currentSample;
    float morphValue; // 0.0 = sharp exp, 0.5 = linear, 1.0 = soft curve

    float applyMorphShape(float t)
    {
        // Clamp t just to be safe
        t = std::clamp(t, 0.0f, 1.0f);

        // Envelope shapes
        float expCurve = std::pow(t, 6.0f); // Sharp exponential
        float expo = std::pow(t, 3.0f); // Exponential
        float linear = t; // Linear
        float logCurve = 1.0f - std::pow(1.0f - t, 2.0f); // Logarithmic
        float softCurve = std::sqrt(t); // Concave

        // Interpolation morphing
        float position = morphValue * 4.0f;
        int idx = static_cast<int>(position);
        float frac = position - idx;

        float a = 0.0f;
        float b = 0.0f;

        switch (idx) {
        case 0:
            a = expCurve;
            b = expo;
            break;
        case 1:
            a = expo;
            b = linear;
            break;
        case 2:
            a = linear;
            b = logCurve;
            break;
        case 3:
            a = logCurve;
            b = softCurve;
            break;
        default:
            return softCurve; // edge case
        }

        return a + (b - a) * frac;
    }
};
