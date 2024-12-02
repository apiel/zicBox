#ifndef _AUDIO_UTILS_H_
#define _AUDIO_UTILS_H_

#include <cstdint>

float linearInterpolation(float index, uint16_t lutSize, float* lut)
{
    // Linear Interpolation to get smoother transitions between discrete LUT values
    float lutIndex = index * (lutSize - 1);
    uint16_t index1 = (uint16_t)lutIndex;
    uint16_t index2 = (index1 + 1) % lutSize;
    float fractional = lutIndex - index1;

    return lut[index1] * (1.0f - fractional) + lut[index2] * fractional;
}

// float linearInterpolation(float x, uint16_t lutSize, float* lut)
// {
//     float scaledIndex = x * (lutSize - 1);
//     int index = static_cast<int>(scaledIndex);
//     float fraction = scaledIndex - index;

//     float value1 = lut[index];
//     float value2 = lut[(index + 1) % lutSize];
//     return value1 + fraction * (value2 - value1);
// }

#endif