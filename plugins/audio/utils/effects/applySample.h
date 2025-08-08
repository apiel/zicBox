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

float applyTremolo(float input, float amount, float& tremoloPhase)
{
    if (amount == 0.0f) {
        return input;
    }

    float speed = 1.0f; // Tremolo speed in Hz
    tremoloPhase += 0.05f * speed;
    float mod = (sin(tremoloPhase) + 1.0f) / 2.0f; // Modulation between 0-1

    return input * (1.0f - amount + amount * mod);
}

float applyRingMod(float input, float amount, float& ringPhase, float sampleRate)
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

    float modulator = sin(ringPhase); // Sine wave oscillator
    float modulated = input * modulator; // Apply ring modulation

    return (1.0f - amount) * input + amount * modulated;
}