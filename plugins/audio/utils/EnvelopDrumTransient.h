#pragma once
#include <algorithm>
#include <cmath>

class EnvelopDrumTransient {
public:
    EnvelopDrumTransient()
        : sampleCount(1)
        , currentSample(0)
        , morphValue(0.5f)
        , maxAmp(0.7f)
    {
    }

    void morph(float value)
    {
        morphValue = std::clamp(value, 0.0f, 1.0f);
    }

    void setMaxAmp(float amp)
    {
        maxAmp = std::clamp(amp, 0.0f, 2.0f); // 1.0f default
    }

    void reset(int durationSamples)
    {
        sampleCount = std::max(1, durationSamples);
        currentSample = 0;
    }

    float next()
    {
        if (currentSample >= sampleCount)
            return 0.0f;

        float t = static_cast<float>(currentSample++) / sampleCount;
        return maxAmp * shape(t);
    }

    float dataPtr;
    float* getMorphShape(float pct)
    {
        dataPtr = -shape(1 - pct);
        return &dataPtr;
    }

private:
    int sampleCount;
    int currentSample;
    float morphValue;
    float maxAmp;

    float shape(float t)
    {
        float p = morphValue * 5.0f;
        int idx = static_cast<int>(p);
        float f = p - idx;

        float a = shapeFunc(idx, t);
        float b = shapeFunc(idx + 1, t);

        return a + (b - a) * f;
    }

    float shapeFunc(int index, float t)
    {
        t = std::clamp(t, 0.0f, 1.0f);

        switch (index) {
        case 0:
            return (t < 0.05f) ? 1.0f - (t * 20.0f) : 0.0f; // Impulse spike
        case 1:
            return std::pow(t, 0.2f) * (1.0f - t); // Punchy attack, fast decay
        case 2:
            return 1.0f - std::abs(2.0f * t - 1.0f); // Triangle blip
        case 3:
            return std::sin(t * 3.1415f) * (1.0f - t); // Overshoot and pull
        case 4:
            return std::exp(-10.0f * t); // Taily snare envelope
        case 5:
            return std::sin(t * 3.1415f); // Full sine blip
        default:
            return 0.0f;
        }
    }
};
