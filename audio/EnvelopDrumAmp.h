/** Description:
This header file acts as a specialized toolkit for controlling how the volume, or amplitude, of a sound—specifically a drum sound—fades in and out over time. This controlled change over time is often called an "envelope."

### Core Functionality

The main purpose of the code is to define and select various **shaping curves**. These mathematical functions determine the feel of the fade:

1.  **Curve Definition:** Several standard curves are defined, such as:
    *   **Linear:** A straight, steady rate of change.
    *   **Logarithmic/Cubic:** Changes that start slowly and accelerate rapidly.
    *   **Sine/Square Root:** Changes that follow smoother, more organic paths.
2.  **The Shape Menu:** All seven available mathematical curves (including predefined exponential curves from another file) are collected into a static menu. This menu serves as the full selection of amplitude behaviors available to the system.
3.  **Specialized Envelope:** A unique control object named `EnvelopDrumAmp` is created. This object is specialized for drum amplitude (volume) changes. When it is initialized, it is automatically configured to use the defined menu of seven curves.

In summary, this file provides the specific library of dynamic fade-in and fade-out behaviors that a drum synthesis engine can instantly select and apply to make sounds feel snappy, gentle, or aggressively shaped.

sha: 3bd80facea60c8b05d3719feba6f244275a675bbd6e266e1f94328f8771ad6e8 
*/
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
