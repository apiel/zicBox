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

void applyGain(float* buffer, uint64_t count)
{
    float max = 0.0;
    for (uint64_t i = 0; i < count; i++) {
        if (buffer[i] > max) {
            max = buffer[i];
        } else if (-buffer[i] > max) {
            max = -buffer[i];
        }
    }
    float gain = 1.0 / max;
    // printf("max sample: %f gain: %f\n", max, gain);
    for (uint64_t i = 0; i < count; i++) {
        buffer[i] = buffer[i] * gain;
    }
}

void limitBuffer(float* buffer, uint64_t count, float limit)
{
    float max = 0.0;
    for (uint64_t i = 0; i < count; i++) {
        if (buffer[i] > max) {
            max = buffer[i];
        } else if (-buffer[i] > max) {
            max = -buffer[i];
        }
    }

    if (max > limit) {
        float ratio = limit / max;
        for (uint64_t i = 0; i < count; i++) {
            buffer[i] = buffer[i] * ratio;
        }
    }
}
