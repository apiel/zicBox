#pragma once

#include "audio/lut/tanhTable.h"

static constexpr int TANH_MASK = TANH_TABLE_SIZE - 1;
inline float getTanh(float x)
{
    if (x <= -5.0f) return -1.0f;
    if (x >= 5.0f) return 1.0f;
    float pos = (x + 5.0f) * (TANH_MASK) * 0.1f;
    int i = (int)pos;
    float frac = pos - (float)i;
    return TANH_TABLE[i] + frac * (TANH_TABLE[i + 1] - TANH_TABLE[i]);
}