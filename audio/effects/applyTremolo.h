#pragma once

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