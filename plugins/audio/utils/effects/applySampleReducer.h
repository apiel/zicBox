#pragma once
#include <cmath>

float applySampleReducer(float input, float amount, float& sampleSqueeze, int& samplePosition)
{
    if (amount == 0.0f) {
        return input;
    }
    if (samplePosition < amount * 100 * 2) {
        samplePosition++;
    } else {
        samplePosition = 0;
        sampleSqueeze = input;
    }

    return sampleSqueeze;
}

float applyBitcrusher(float input, float amount, float& sampleHold, int& sampleCounter)
{
    if (amount == 0.0f) {
        return input;
    }

    // Reduce Bit Depth
    int bitDepth = 2 + amount * 10; // Stronger effect
    float step = 1.0f / (1 << bitDepth); // Quantization step
    float crushed = round(input / step) * step; // Apply bit reduction

    // Reduce Sample Rate
    int sampleRateDivider = 1 + amount * 20; // Reduces update rate
    if (sampleCounter % sampleRateDivider == 0) {
        sampleHold = crushed; // Hold the value for "stepping" effect
    }
    sampleCounter++;

    if (amount < 0.1f) {
        // mix with original signal
        return sampleHold * (amount * 10) + input * (1.0f - (amount * 10));
    }

    return sampleHold;
}