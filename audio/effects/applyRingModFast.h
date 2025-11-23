/** Description:
## Digital Audio Effect Analysis: Fast Ring Modulation

This header code defines a high-speed audio processing function designed to apply a specific digital sound effect known as Ring Modulation. This effect is commonly used in sound design to create robotic, metallic, or dissonant bell-like textures.

### I. Purpose

The function’s primary job is to take a single moment (sample) of an audio signal and blend it with a newly generated, hidden tone, transforming the sound based on how intensely the effect is applied. The "Fast" designation indicates the use of optimized mathematical shortcuts, essential for effects that must run smoothly in real-time audio systems.

### II. Basic Idea of Operation

Ring Modulation works by multiplying the incoming audio signal by a simple, repeating wave, usually a sine wave (the "modulator").

1.  **Frequency Control:** The strength of the effect determines the frequency of this hidden tone. For example, a low effect strength might use a 200 Hz tone, while maximum strength could push the tone up to 1000 Hz.
2.  **Wave Tracking:** The function constantly tracks the exact position of the modulating wave over time (the "phase") to ensure the tone remains smooth and continuous between individual audio measurements.
3.  **Signal Mixing:** The core modulation step involves a direct multiplication of the input sound and the generated tone.
4.  **Dry/Wet Blend:** Finally, the function mixes the fully processed (wet) signal with the original (dry) input audio based on the effect's strength setting. If the strength is zero, the sound is completely bypassed and returned untouched.

This mixing technique is crucial for achieving complex, frequency-shifting sonic results.

sha: c685b0644646f9ba77694e517fbca0719b8b3d9005119805300d1551611f1570 
*/
#pragma once

#include "helpers/math.h"

float applyRingModFast(float input, float amount, float& ringPhase, float sampleRate)
{
    if (amount == 0.0f) {
        return input;
    }

    float ringFreq = 200.0f + amount * 800.0f; // Modulation frequency (200Hz - 1000Hz)
    ringPhase += PI_X2 * ringFreq / sampleRate;

    // Keep phase in the [0, 2π] range
    if (ringPhase > PI_X2) {
        ringPhase -= PI_X2;
    }

    float modulator = fastSin2(ringPhase); // Sine wave oscillator
    float modulated = input * modulator; // Apply ring modulation

    return (1.0f - amount) * input + amount * modulated;
}