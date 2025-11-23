/** Description:
This code defines a specialized mathematical function used in digital signal processing, specifically to create the audio effect known as **Ring Modulation**.

### Purpose and Functionality

Ring Modulation is a classic sound effect often used in synthesis to generate metallic, robotic, or dissonant sounds.

The function takes an incoming audio sample and modifies it based on several controls:
1.  **Input Audio:** The original sound signal.
2.  **Amount:** A control slider (from 0% to 100%) that determines the intensity of the effect.
3.  **Sample Rate:** Technical information about the quality and speed of the audio stream.

### How the Code Works (Basic Idea)

At its core, Ring Modulation operates by constantly multiplying the input sound by a separate, invisible sine wave oscillator.

1.  **Frequency Control:** The function first calculates the speed (frequency) of this hidden oscillator. This frequency is tied directly to the **Amount** control—the more effect requested, the higher and faster the internal sine wave moves (ranging from about 200 Hz to 1000 Hz).
2.  **Oscillator Management:** It tracks the current position (the phase) of this internal sine wave, ensuring its movement is continuous and smooth, regardless of the audio stream speed.
3.  **Modulation:** The resulting sine wave value is mathematically multiplied with the original audio sample. This multiplication is what creates the distinct, harmonically complex sound of the effect.
4.  **Mixing:** Finally, the function blends the processed (modulated) audio with the original, unaffected sound. If the **Amount** is set low, the original sound dominates; if set high, the fully processed, robotic sound is heard.

sha: d16fb4a68ea4a3e18ddef8e4f92511e0f4f78e838a71a59135ff6b502c8d3e43 
*/
#pragma once
#include <cmath>

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