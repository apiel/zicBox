#pragma once

#include <math.h>

float applyCompression(float input, float compressAmount)
{
    if (compressAmount == 0.0f) {
        return input;
    }
    // How about?
    // return (input * (1 - compressAmount)) + (range(std::pow(input, compressAmount * 0.8f), -1.0f, 1.0f) * fxAmount.pct());
    if (input > 0.0f) {
        return std::pow(input, 1.0f - compressAmount * 0.8f);
    }
    return -std::pow(-input, 1.0f - compressAmount * 0.8f);
}
