#pragma once

float applyHighFreqBoost2(float input, float amount, float& boostTimeInc, float& boostTime)
{
    if (boostTimeInc == 0) {
        return input;
    }
    float highFreqComponent = input * boostTime ; // Emphasize high frequencies
    boostTime += boostTimeInc;
    return input + highFreqComponent * amount;
}
