#pragma once
#include "EnvelopMorph.h"
#include <cmath>

namespace EnvelopeShapes {

inline float impulse(float t) { return (t < 0.05f) ? 1.0f - (t * 20.0f) : 0.0f; }
inline float punchy(float t) { return std::pow(t, 0.2f) * (1.0f - t); }
inline float triangle(float t) { return 1.0f - std::abs(2.0f * t - 1.0f); }
inline float overshoot(float t) { return std::sin(t * 3.1415f) * (1.0f - t); }
inline float snareTail(float t) { return std::exp(-10.0f * t); }
inline float sineBlip(float t) { return std::sin(t * 3.1415f); }

constexpr std::array<EnvelopeShapeFunc, 6> transientShapes = {
    impulse, punchy, triangle, overshoot, snareTail, sineBlip
};

} // namespace EnvelopeShapes

class EnvelopDrumTransient : public EnvelopMorph {
public:
    EnvelopDrumTransient()
        : maxAmp(0.7f)
    {
        shapeFuncs = EnvelopeShapes::transientShapes.data();
        shapeCount = static_cast<int>(EnvelopeShapes::transientShapes.size());
    }

    void setMaxAmp(float amp)
    {
        maxAmp = std::clamp(amp, 0.0f, 2.0f);
    }

protected:
    float maxAmp;

    float transformTime(float t) const override
    {
        return t; // Transient uses natural time (0 → 1)
    }

    float scaleOutput(float value) const override
    {
        return maxAmp * value;
    }
};
