#pragma once

#include <cstdint>

float linearInterpolationAbsolute(float index, uint16_t lutSize, float* lut)
{
    uint16_t index1 = static_cast<uint16_t>(index);
    uint16_t index2 = (index1 + 1) % lutSize;
    float fractional = index - index1;

    // Interpolate between the two LUT values
    return lut[index1] * (1.0f - fractional) + lut[index2] * fractional;
}

// Linear interpolation relative where index is between 0.0f and 1.0f
float linearInterpolation(float index, uint16_t lutSize, float* lut)
{
    return linearInterpolationAbsolute(index * (lutSize - 1), lutSize, lut);
}
