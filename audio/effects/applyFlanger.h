#pragma once

#include <cmath>

float applyFlanger(float input, float amount, float* buffer, int& bufferIndex, int bufferSize, float& flangerPhase)
{
    if (amount == 0.0f) {
        return input;
    }

    int min_delay = 144; // 3ms at 48000khz
    int max_delay = 1200; // 25ms at 48000khz
    float speed = 1.0f; // Flanger speed in Hz
    flangerPhase += 0.00002f * speed;
    float mod = (sinf(flangerPhase) + 1.0f) / 2.0f; // Modulation between 0-1

    int delay = mod * max_delay + min_delay;
    int readIndex = (bufferIndex + bufferSize - delay) % bufferSize;

    float out = input + buffer[readIndex] * amount;

    buffer[bufferIndex] = out;
    bufferIndex = (bufferIndex + 1) % bufferSize;

    return out;
}
