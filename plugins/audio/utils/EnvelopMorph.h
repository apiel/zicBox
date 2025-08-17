#pragma once
#include <algorithm>
#include <cmath>
#include <functional>
#include <array>

using EnvelopeShapeFunc = float(*)(float);

class EnvelopMorph {
public:
    EnvelopMorph()
        : sampleCount(1)
        , currentSample(0)
        , morphValue(0.5f)
        , shapeFuncs(nullptr)
        , shapeCount(0)
    {}

    virtual ~EnvelopMorph() = default;

    void morph(float value) {
        morphValue = std::clamp(value, 0.0f, 1.0f);
    }

    void reset(int durationSamples) {
        sampleCount = std::max(1, durationSamples);
        currentSample = 0;
    }

    float* getMorphShape(float pct) {
        float t = std::clamp(1.0f - pct, 0.0f, 1.0f);
        dataPtr = std::clamp(-evaluateShape(t), -1.0f, 1.0f);
        return &dataPtr;
    }

    float next() {
        if (currentSample >= sampleCount)
            return 0.0f;

        float t = static_cast<float>(currentSample++) / sampleCount;
        return scaleOutput(evaluateShape(transformTime(t)));
    }

    float next(float t) {
        return scaleOutput(evaluateShape(transformTime(t)));
    }

protected:
    int sampleCount;
    int currentSample;
    float morphValue;
    float dataPtr;

    const EnvelopeShapeFunc* shapeFuncs;
    int shapeCount;

    virtual float transformTime(float t) const {
        return 1.0f - t; // default (amp envelope style)
    }

    virtual float scaleOutput(float value) const {
        return value; // default: no scaling
    }

    float evaluateShape(float t) const {
        if (!shapeFuncs || shapeCount == 0)
            return 0.0f;

        float pos = morphValue * (shapeCount - 1);
        int idx = static_cast<int>(pos);
        float frac = pos - idx;

        float a = shapeFuncs[std::min(idx, shapeCount - 1)](t);
        float b = shapeFuncs[std::min(idx + 1, shapeCount - 1)](t);
        return a + (b - a) * frac;
    }
};
