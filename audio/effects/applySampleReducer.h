#pragma once

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