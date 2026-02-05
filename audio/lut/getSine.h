#pragma once

#include "audio/lut/sineTable.h"

static constexpr int SINE_MASK = SINE_TABLE_SIZE - 1;
inline float getSine(float phase)
{
    float pos = phase * SINE_TABLE_SIZE;
    int i = (int)pos;
    float frac = pos - (float)i;
    int i1 = i & SINE_MASK;
    int i2 = (i1 + 1) & SINE_MASK;
    return SINE_TABLE[i1] + frac * (SINE_TABLE[i2] - SINE_TABLE[i1]);
}