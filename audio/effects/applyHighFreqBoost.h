#pragma once

float applyHighFreqBoost(float input, double& boostTimeInc, double& boostTime)
{
    if (boostTimeInc == 0) {
        return input;
    }
    // Simple high-shelf boost logic
    // TODO optimize precalculate: highBoost.get() * time
    // float highFreqComponent = input * (highBoost.get() * boostTime); // Emphasize high frequencies
    float highFreqComponent = input * boostTime; // Emphasize high frequencies
    boostTime += boostTimeInc;
    return input + highFreqComponent;
}
