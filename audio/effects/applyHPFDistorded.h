#pragma once

#include <cmath>

float applyHPFDistorded(float input, float amount, int sampleRate, float& hp_z1, float& lp_z1)
{
    if (amount <= 0.0f)
        return input;

    amount = 1.0f - powf(amount, 0.25f);

    float cutoff = 200.0f + (sampleRate * 0.48f - 200.0f) * amount;
    float alpha = sampleRate / (cutoff + sampleRate);
    hp_z1 = alpha * (hp_z1 + input - (lp_z1 = lp_z1 + (cutoff / (cutoff + sampleRate)) * (input - lp_z1)));
    return hp_z1;
}