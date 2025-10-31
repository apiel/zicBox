#pragma once

float applyDecimator(float input, float amount, float& decimHold, int& decimCounter)
{
    int interval = 1 + (int)(amount * 30.0f); // Downsample ratio
    if (decimCounter % interval == 0) {
        decimHold = input;
    }
    decimCounter++;
    return decimHold;
}
