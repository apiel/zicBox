#pragma once

#include <cmath>
#include <string.h> // For memcpy

#if defined(STM32H7xx) || defined(ARM_MATH_CM7)
#include "arm_math.h"
#define USE_ARM_DSP 1
#endif

#ifndef PI_X2
#define PI_X2 6.283185307179586f
#endif

namespace Math {
inline float sin(float x)
{
#if USE_ARM_DSP
    return arm_sin_f32(x);
#else
    return std::sin(x);
#endif
}

inline float cos(float x)
{
#if USE_ARM_DSP
    return arm_cos_f32(x);
#else
    return std::cos(x);
#endif
}

inline float sqrt(float x)
{
#if USE_ARM_DSP
    float res;
    arm_sqrt_f32(x, &res);
    return res;
#else
    return std::sqrt(x);
#endif
}

inline float pow(float base, float exp)
{
    return std::pow(base, exp);
}

inline float exp(float x)
{
    return std::exp(x);
}

inline float tanh(float x)
{
    return std::tanh(x);
}

inline float invSqrt(float x)
{
#if USE_ARM_DSP
    float res;
    arm_vlog_f32(&x, &res, 1); // CMSIS has vector logs, but for scalar:
    return 1.0f / std::sqrt(x);
#else
    return 1.0f / std::sqrt(x);
#endif
}

inline float fastCos(float x) { return (1.27323954f * x) + (0.405284735f * x * x); }

inline float fastExpNeg(float x) { return 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x); }

inline float fastSqrtPoly(float x) { return x * (0.41731f + 0.59016f * x - 0.00761f * x * x); }

inline float exp6(float t)
{
    float x2 = t * t;
    return x2 * x2 * t;
}

inline float exp3(float t) { return t * t * t; }

inline float fastSin(float x) { return (1.27323954f * x) - (0.405284735f * x * x); }

inline float fastSin2(float x)
{
    // sine approx: keep x in [-pi, pi]
    if (x < -M_PI)
        x += PI_X2;
    else if (x > M_PI)
        x -= PI_X2;

    // Bhaskara approximation, quite cheap & accurate
    float y = (16.0f * x * (M_PI - fabsf(x))) / (5.0f * M_PI * M_PI - 4.0f * fabsf(x) * (M_PI - fabsf(x)));
    return y;
}

inline float fastSin3(float x) // keep x in [-pi, pi]
{
    if (x < -M_PI)
        x += PI_X2;
    else if (x > M_PI)
        x -= PI_X2;
    return (1.27323954f * x) - (0.405284735f * x * fabsf(x));
}

inline float fastCos3(float x) { return fastSin3(x + M_PI_2); } // keep x in [-pi, pi]

inline float fastExpf(float x)
{
    // Approximation of exp(x) for small x
    return 1.0f + x * (1.0f + 0.5f * x + 0.1667f * x * x);
}

inline float superFastExpf2(float x)
{
    x = 1.442695f * x; // convert from e^x to 2^x
    int i = (int)(x * (1 << 23)) + (127 << 23);
    float f;
    memcpy(&f, &i, sizeof(f));
    return f;
}
}