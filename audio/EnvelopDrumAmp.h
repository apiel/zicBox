#pragma once

#include "./EnvelopMorph.h"
#include "helpers/math.h"

namespace EnvelopShapes {

inline float linear(float t) { return t; }
inline float logCurve(float t) { float x = 1.0f - t; return 1.0f - (x * x); }
inline float sqrt(float t) { return fastSqrtPoly(t); }
inline float sine(float t) { return fastSin(t * 1.57079632679f); }
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
