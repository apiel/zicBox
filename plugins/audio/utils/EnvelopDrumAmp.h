#pragma once

#include "./EnvelopMorph.h"

namespace EnvelopShapes {

inline float exp6(float t) { return std::pow(t, 6.0f); }
inline float exp3(float t) { return std::pow(t, 3.0f); }
inline float linear(float t) { return t; }
inline float logCurve(float t) { return 1.0f - std::pow(1.0f - t, 2.0f); }
inline float sqrt(float t) { return std::sqrt(t); }
inline float sine(float t)
{
    return (1.27323954f * t * 1.57079632679f) - (0.405284735f * t * t * 1.57079632679f);
}
inline float cubicEase(float t)
{
    float inv = 1.0f - t;
    return 1.0f - inv * inv * inv;
}

constexpr std::array<EnvelopeShapeFunc, 7> ampShapes = {
    exp6, exp3, linear, logCurve, sqrt, sine, cubicEase
};

} // namespace EnvelopShapes

class EnvelopDrumAmp : public EnvelopMorph {
public:
    EnvelopDrumAmp()
    {
        shapeFuncs = EnvelopShapes::ampShapes.data();
        shapeCount = static_cast<int>(EnvelopShapes::ampShapes.size());
    }
};
