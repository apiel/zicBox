#pragma once

#include <cstdint>

void applySampleGain(float* buffer, uint64_t count)
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
