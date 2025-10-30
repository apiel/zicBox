#pragma once

#include "EnvelopMorph.h"
#include "helpers/math.h"

#include <cmath>

namespace EnvelopeShapes {

inline float linear(float t) { return t; }
inline float impulse(float t) { return (t < 0.05f) ? 1.0f - (t * 20.0f) : 0.0f; }
inline float punchy(float t) { return t * t * (1.0f - t); }
inline float triangle(float t) { return 1.0f - std::abs(2.0f * t - 1.0f); }
inline float overshoot(float t) { return fastSin(t * 3.1415f) * (1.0f - t); }
inline float snareTail(float t) { return fastExpNeg(10.0f * t); }
inline float sineBlip(float t) { return fastSin(t * 3.1415f); }

inline float pulseTrain(float t)
{
    float decay = fastExpNeg(10.0f * t);
    float pulse = (std::fmod(t * 20.0f, 1.0f) < 0.5f) ? 1.0f : 0.0f; // 10 pulses
    return decay * pulse;
}
inline float pulseTrainDecay(float t)
{
    return pulseTrain(1.0f - t);
}

constexpr std::array<EnvelopeShapeFunc, 11> transientShapes = {
    exp6, exp3, linear, impulse, punchy, triangle, overshoot, snareTail, sineBlip, pulseTrainDecay, pulseTrain
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
