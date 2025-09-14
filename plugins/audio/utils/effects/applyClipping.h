#pragma once

#include "helpers/clamp.h"

float applyClipping(float input, float scaledClipping)
{
    if (scaledClipping == 0.0f) {
        return input;
    }
    return CLAMP(input + input * scaledClipping, -1.0f, 1.0f);
}
