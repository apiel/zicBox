#pragma once

float applyBoost(float input, float bassBoostAmount, float& prevInput, float& prevOutput)
{
    if (bassBoostAmount == 0.0f) {
        return input;
    }
    float bassFreq = 0.2f + 0.8f * bassBoostAmount;
    float bassBoosted = (1.0f - bassFreq) * prevOutput + bassFreq * (input + prevInput) * 0.5f;
    prevInput = input;
    prevOutput = bassBoosted;
    bassBoosted *= 1.0f + bassBoostAmount * 2.0f;

    return bassBoosted;
}
