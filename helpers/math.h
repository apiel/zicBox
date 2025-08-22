#pragma once

#include <math.h>


inline float fastSin(float x) { return (1.27323954f * x) - (0.405284735f * x * x); }

inline float fastCos(float x) { return (1.27323954f * x) + (0.405284735f * x * x); }

inline float fastExpNeg(float x) { return 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x); }

inline float fastSqrtPoly(float x) { return x * (0.41731f + 0.59016f * x - 0.00761f * x * x); }

inline float exp6(float t)
{
    float x2 = t * t;
    return x2 * x2 * t;
}

inline float exp3(float t) { return t * t * t; }

float fastSin2(float x)
{
    // sine approx: keep x in [-pi, pi]
    if (x < -M_PI)
        x += 2 * M_PI;
    else if (x > M_PI)
        x -= 2 * M_PI;

    // Bhaskara approximation, quite cheap & accurate
    float y = (16.0f * x * (M_PI - fabsf(x))) / (5.0f * M_PI * M_PI - 4.0f * fabsf(x) * (M_PI - fabsf(x)));
    return y;
}