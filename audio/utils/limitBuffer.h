#pragma once

#include <cstdint>

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
