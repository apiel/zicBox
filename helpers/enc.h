#pragma once

#include <cmath>
#include <cstdint>

#include "helpers/clamp.h"

int encGetScaledDirection(int8_t direction, uint32_t tick, uint32_t lastTick)
{
    // Calculate time difference between ticks
    uint32_t delta = tick - lastTick;

    if (delta == 0)
        delta = 1;

    // Compute a speed factor: smaller delta = faster turn
    // Example: if delta = 200ms → factor = 1
    //          if delta = 10ms  → factor = 10
    // The constants can be tuned
    float speedFactor = 200.0f / delta;

    // Clamp to a reasonable range (1x to 10x)
    speedFactor = CLAMP(speedFactor, 1.0f, 10.0f);

    // Apply scaled direction (round to int)
    int scaledDirection = direction * (int)round(speedFactor);
    return scaledDirection;
}