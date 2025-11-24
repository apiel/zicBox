/** Description:
This file serves as a specialized library containing extremely fast, optimized approximations for common mathematical functions. Its primary purpose is to provide high-speed calculations, often at the expense of perfect numerical precision.

The core idea is to replace complex, slow calculations (like those found in standard programming libraries) with simplified polynomial formulas. This approach is critical for performance-intensive applications, such as real-time simulations, video game engines, or audio processing, where millions of calculations must occur instantaneously.

The header includes quick approximations for:

1.  **Trigonometry:** Functions that estimate Sine (`fastSin`) and Cosine (`fastCos`). These use efficient algebraic expressions to model the wave patterns, often restricting the input to a small range (like -180 to 180 degrees) for better accuracy.
2.  **Exponentials:** Rapid methods for calculating the exponential function ($e^x$). One unique function achieves extreme speed by manipulating the underlying binary representation of the floating-point numbers directly—a low-level hardware trick designed purely for maximum velocity.
3.  **Square Roots and Powers:** Simple, fast estimates for square root and specific power functions.

In summary, this collection provides a crucial performance boost by favoring calculation speed over absolute mathematical exactness.

sha: 0689b390f1eb977a605059dbb9bee777d532723132f048460b0f98b28493ff09 
*/
#pragma once

#include <math.h>
#include <string.h>

#ifndef PI_X2
#define PI_X2 6.283185307179586f
#endif

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
        x += PI_X2;
    else if (x > M_PI)
        x -= PI_X2;

    // Bhaskara approximation, quite cheap & accurate
    float y = (16.0f * x * (M_PI - fabsf(x))) / (5.0f * M_PI * M_PI - 4.0f * fabsf(x) * (M_PI - fabsf(x)));
    return y;
}

float fastSin3(float x) // keep x in [-pi, pi]
{
    if (x < -M_PI)
        x += PI_X2;
    else if (x > M_PI)
        x -= PI_X2;
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