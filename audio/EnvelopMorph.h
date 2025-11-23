/** Description:
This code defines a sophisticated utility called `EnvelopMorph`, which generates continuous, time-based control signals, commonly referred to as "envelopes" in digital signal processing (like managing volume fades or filter sweeps).

### Core Functionality

The main purpose of this system is **morphing**. It allows the signal generator to seamlessly transition between two or more pre-defined envelope shapes.

1.  **Shape Blending:** The system maintains a `morphValue` (a percentage between 0 and 1) that determines the blend ratio. For example, if you have Shape A and Shape B, a 0.5 morph value means the output is an equal mix of both A and B. The internal logic calculates this exact blend at every moment in time.

2.  **Time Progression:** The envelope is managed using samples (time steps). The `reset` function defines the total duration. The primary function, `next()`, advances the internal time counter and calculates the precise output value required by the blended shape for that specific time step.

3.  **Customization:** The system is designed to be flexible. It includes built-in hooks that allow developers to modify how time progresses (e.g., playing the envelope backward) or to apply special scaling or adjustments to the final output value before it is returned.

In essence, `EnvelopMorph` provides a dynamic framework for creating complex, controllable movement patterns that evolve smoothly over a defined duration by mixing multiple base shapes.

sha: a24caeee293f0f865ed3d4bdfa66402f98cc46e9f71ea90849092821e03cef33 
*/
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

    float getMorph() const {
        return morphValue;
    }

    void reset(int durationSamples) {
        sampleCount = std::max(1, durationSamples);
        currentSample = 0;
    }

    float* getMorphShape(float pct) {
        float t = std::clamp(1.0f - pct, 0.0f, 1.0f);
        dataPtr = std::clamp(evaluateShape(t), -1.0f, 1.0f);
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
