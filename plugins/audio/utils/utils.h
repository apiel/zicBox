#ifndef _AUDIO_UTILS_H_
#define _AUDIO_UTILS_H_

#include <cstdint>

float linerInterpolation(float index, uint16_t lutSize, float* lut)
{
    // Linear Interpolation to get smoother transitions between discrete LUT values
    float lutIndex = index * lutSize;
    uint16_t index1 = (uint16_t)lutIndex;
    uint16_t index2 = (index1 + 1) % lutSize;
    float fractional = lutIndex - index1;

    return lut[index1] * (1.0f - fractional) + lut[index2] * fractional;
}

#endif