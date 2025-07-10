#pragma once

#include "helpers/range.h"

float applyClipping(float input, float scaledClipping)
{
    if (scaledClipping == 0.0f) {
        return input;
    }
    return range(input + input * scaledClipping, -1.0f, 1.0f);
}
