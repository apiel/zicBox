#pragma once

#include <cstdint>

namespace Noise {
// Pre-calculate the reciprocal of 4294967295.0
static constexpr float invUintMax = 1.0f / 4294967295.0f;
uint32_t y = 123456789; // Seed
float get()
{
    y ^= (y << 13);
    y ^= (y >> 17);
    y ^= (y << 5);
    return ((float)y * invUintMax);
}

float sample()
{
    return get() * 2.0f - 1.0f;
}

}
