#pragma once

#include "helpers/math.h"

float applyRingModFast(float input, float amount, float& ringPhase, float sampleRate)
{
    if (amount == 0.0f) {
        return input;
    }

    float ringFreq = 200.0f + amount * 800.0f; // Modulation frequency (200Hz - 1000Hz)
    ringPhase += 2.0f * M_PI * ringFreq / sampleRate;

    // Keep phase in the [0, 2π] range
    if (ringPhase > 2.0f * M_PI) {
        ringPhase -= 2.0f * M_PI;
    }

    float modulator = fastSin(ringPhase); // Sine wave oscillator
    float modulated = input * modulator; // Apply ring modulation

    return (1.0f - amount) * input + amount * modulated;
}