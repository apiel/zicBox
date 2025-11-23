/** Description:
This C++ header file defines a specialized function designed for digital audio processing. Its primary purpose is to implement the **Tremolo** effect.

Tremolo is a classic audio effect that causes the perceived volume of a sound to cyclically increase and decrease, creating a rhythmic, pulsating texture, much like someone quickly turning a volume knob up and down.

The function, named `applyTremoloFast`, takes the incoming audio signal and modifies its strength. It works by generating a continuous, smooth internal "wave" (mathematically calculated using a sine function) that acts as a modulator.

This modulating wave controls how much the volume is changed. A key parameter determines the "amount" or depth of the effect. If this amount is set to zero, the function ensures the original sound passes through completely unchanged.

The function is structured to be continuous: it maintains an internal tracker (called the phase) that remembers where the volume cycle left off, ensuring the effect is smooth and uninterrupted across the entire audio stream. Essentially, it multiplies the incoming sound sample by a smoothly oscillating volume level to produce the signature tremolo sound.

sha: bb11a46a8ecacafd9efa4bba01fbdbb57700be34eb6be46fb10d727d368609ba 
*/
#pragma once

#include <cmath>

// TODO TODO TODO
float applyTremoloFast(float input, float amount, float& tremoloPhase)
{
    if (amount == 0.0f) {
        return input;
    }

    float speed = 1.0f; // Tremolo speed in Hz
    tremoloPhase += 0.05f * speed;
    float mod = (sinf(tremoloPhase) + 1.0f) / 2.0f; // Modulation between 0-1

    return input * (1.0f - amount + amount * mod);
}