/** Description:
This section of code defines a function responsible for applying a common digital audio effect known as **Tremolo**.

### Overview

Tremolo is a modulation effect that causes a periodic and automatic variation in the volume or amplitude of an incoming sound signal. It simulates the effect of quickly turning a volume knob up and down repeatedly, resulting in a distinct pulsing sound.

### Basic Functionality

The `applyTremolo` function works as a digital sound processor, taking the current sound value (input), and modifying it based on two key controls:

1.  **Amount:** This controls the depth or intensity of the volume fluctuation. If set to zero, the sound passes through unchanged.
2.  **Phase:** This is an internal variable that keeps track of where the effect is in its current cycle, ensuring the volume fluctuation is continuous and smooth over time.

### How It Works

The core idea relies on a **sine wave**, which is a smooth, continuous mathematical curve.

1.  The function generates this internal sine wave, which continuously cycles between its minimum and maximum points.
2.  This wave acts as an automated volume control, ranging from 0 (off) to 1 (full volume).
3.  The function then calculates the final output by multiplying the original sound input by this fluctuating volume level, scaling the intensity based on the set "amount."

This ensures the sound’s volume smoothly rises and falls at a fixed rate (set internally to 1 cycle per second), creating the signature trembling effect.

sha: e2e877a6e214c135017ae32e41dfab0dd287553b19b5c0f1a68b18509bfa9739 
*/
#pragma once

#include <cmath>

float applyTremolo(float input, float amount, float& tremoloPhase)
{
    if (amount == 0.0f) {
        return input;
    }

    float speed = 1.0f; // Tremolo speed in Hz
    tremoloPhase += 0.05f * speed;
    float mod = (sinf(tremoloPhase) + 1.0f) / 2.0f; // Modulation between 0-1

    return input * (1.0f - amount + amount * mod);
}