#pragma once

#include <cmath>

float applyBandpass(float x, float center, float Q, int sampleRate, float& bp_x1, float& bp_x2, float& bp_y1, float& bp_y2)
{
    float omega = 2.f * M_PI * center / sampleRate;
    float alpha = sinf(omega) / (2.f * Q);

    float b0 = alpha;
    float b1 = 0.f;
    float b2 = -alpha;
    float a0 = 1.f + alpha;
    float a1 = -2.f * cosf(omega);
    float a2 = 1.f - alpha;

    float y = (b0 / a0) * x + (b1 / a0) * bp_x1 + (b2 / a0) * bp_x2
        - (a1 / a0) * bp_y1 - (a2 / a0) * bp_y2;

    bp_x2 = bp_x1;
    bp_x1 = x;
    bp_y2 = bp_y1;
    bp_y1 = y;

    return y;
}