#pragma once

#include <math.h>
#include <string.h>

float fastCos(float x) { return (1.27323954f * x) + (0.405284735f * x * x); }

float fastExpNeg(float x) { return 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x); }

float fastSqrtPoly(float x) { return x * (0.41731f + 0.59016f * x - 0.00761f * x * x); }

float exp6(float t)
{
    float x2 = t * t;
    return x2 * x2 * t;
}

float exp3(float t) { return t * t * t; }

float fastSin(float x) { return (1.27323954f * x) - (0.405284735f * x * x); }

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

float fastSin3(float x) // keep x in [-pi, pi]
{
    if (x < -M_PI)
        x += 2.f * M_PI;
    else if (x > M_PI)
        x -= 2.f * M_PI;
    return (1.27323954f * x) - (0.405284735f * x * fabsf(x));
}

float fastCos3(float x) { return fastSin3(x + M_PI_2); } // keep x in [-pi, pi]

float fastExpf(float x)
{
    // Approximation of exp(x) for small x
    return 1.0f + x * (1.0f + 0.5f * x + 0.1667f * x * x);
}

float superFastExpf2(float x) {
    x = 1.442695f * x;  // convert from e^x to 2^x
    int i = (int)(x * (1 << 23)) + (127 << 23);
    float f;
    memcpy(&f, &i, sizeof(f));
    return f;
}