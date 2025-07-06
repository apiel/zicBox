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

    float dataPtr;
    float * getMorphShape(float pct) 
    {
        dataPtr = - applyMorphShape(1 - pct);
        return &dataPtr;
    }

    float applyMorphShape(float t)
    {
        // Clamp t just to be safe
        t = std::clamp(t, 0.0f, 1.0f);

        const int shapeCount = 7; // or however many shapes you have
        float position = morphValue * (shapeCount - 1);
        int idx = static_cast<int>(position);
        float frac = position - idx;

        float a = shapeFunction(idx, t);
        float b = shapeFunction(idx + 1, t);
        return a + (b - a) * frac;
    }

private:
    int sampleCount;
    int currentSample;
    float morphValue; // 0.0 = sharp exp, 0.5 = linear, 1.0 = soft curve

    float shapeFunction(int index, float t)
    {
        switch (index) {
        case 0:
            return expCurve(t); // Sharp exponential
        case 1:
            return expo(t); // Exponential
        case 2:
            return linear(t); // Linear
        case 3:
            return logCurve(t); // Logarithmic
        case 4:
            return softCurve(t); // Concave
        case 5:
            return sineOut(t); // Sine
        case 6:
            return cubicEase(t); // Cubic ease
        default:
            return softCurve(t); // Fallback
        }
    }

    float expCurve(float t)
    {
        return std::pow(t, 6.0f); // Sharp exponential
    }

    float expo(float t)
    {
        return std::pow(t, 3.0f); // Exponential
    }

    float linear(float t)
    {
        return t; // Linear
    }

    float logCurve(float t)
    {
        return 1.0f - std::pow(1.0f - t, 2.0f); // Logarithmic
    }

    float softCurve(float t)
    {
        return std::sqrt(t); // Concave
    }

    // float sineOut(float t)
    // {
    //     return std::sin(t * M_PI_2); // M_PI_2 = pi / 2
    // }
    // Approximate sin(x) over [0, pi/2]
    // float fastSin(float x)
    // {
    //     // x in [0, pi/2]
    //     float x2 = x * x;
    //     // return x * (1.0f - x2 * (1.0f / 6.0f - x2 * (1.0f / 120.0f)));
    //     return x * (1.0f - x2 * (0.1666666666f - x2 * 0.00833333333f));
    // }
    // Ultra fast sin approx
    float fastSin(float x)
    {
        return (1.27323954f * x) - (0.405284735f * x * x);
    }
    float sineOut(float t)
    {
        return fastSin(t * 1.57079632679f); // pi/2
    }

    float cubicEase(float t)
    {
        float inv = 1.0f - t;
        return 1.0f - inv * inv * inv;
    }

    float sigmoid(float t)
    {
        float tt = t * t;
        float inv = (1.0f - t) * (1.0f - t);
        return tt / (tt + inv);
    }
};
