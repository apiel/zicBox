#pragma once

#include <cstdint>

// Pre-calculate the reciprocal of 4294967295.0
static constexpr float invUintMax = 1.0f / 4294967295.0f;
uint32_t y = 123456789; // Seed
float makeNoise() {
    y ^= (y << 13);
    y ^= (y >> 17);
    y ^= (y << 5);
    return ((float)y * invUintMax) * 2.0f - 1.0f;
}


// // Extremely fast, high-quality pseudo-random noise
// uint32_t y = 123456789; // Seed
// float makeNoise() {
//     y ^= (y << 13);
//     y ^= (y >> 17);
//     y ^= (y << 5);
//     return (float)y / (float)UINT32_MAX * 2.0f - 1.0f;
// }
