/** Description:
This file serves as a utility library for audio processing, specifically implementing a technique called **waveshaping**. Waveshaping is used in digital synthesis to intentionally distort or color a sound, adding harmonic content to change its timbre (sound quality).

The core idea is to take an input audio signal and modify it by adding a calculated sine wave component, controlled by a variable called the "waveshape amount."

### How the Code Works

The library provides multiple ways to achieve this modification, balancing computational speed against precision:

1.  **Optimized Waveshaping (Lookup Table):** This function uses a pre-calculated list of sine values, known as a Lookup Table. Instead of calculating the sine value from scratch every time (which uses more CPU power), the system quickly looks up an approximate value and uses a method called linear interpolation to estimate the precise point between the stored entries. This approach is highly efficient for performance-critical applications.

2.  **Standard Waveshaping (Direct Calculation):** This function uses the computer’s highly precise built-in sine function. While offering maximum accuracy, it is generally more computationally demanding than using a lookup table.

3.  **Smart Selector Function:** A final, versatile function allows the user to choose the desired balance simply by adjusting the input value. If the "waveshape amount" is positive, the system uses the standard, highly precise calculation. If the amount is negative, the system automatically switches to the faster, optimized Lookup Table method, prioritizing speed. If the amount is zero, the input sound is left completely unchanged.

In summary, this header file provides specialized tools for creative sound manipulation, giving the audio engine designer flexibility to choose between speed and accuracy depending on the current task.

sha: 9a39e2a1ec29867cbae4b7324499f29a0d37ce7095485738a421ac485604aa71
*/
#pragma once

#include <math.h>

#include "audio/utils/math.h"

float applyWaveshape2(float input, float waveshapeAmount)
{
    float x = input - std::floor(input);
    float normalizedX = (x * 2.0f) - 1.0f;
    // float sineValue = Math::sin(normalizedX * M_PI);
    float sineValue = Math::fastSin3(normalizedX * M_PI);
    return input + (waveshapeAmount * sineValue * 2.0f);
}

// apply waveshape using sinf
float applyWaveshape(float input, float waveshapeAmount)
{
    float sineValue = Math::sin(input);
    return input + waveshapeAmount * sineValue * 2;
}

float applyWaveshape3(float input, float waveshapeAmount)
{
    // The 'drive' increases the number of folds
    float drive = 1.0f + (waveshapeAmount * 10.0f); 
    
    // We wrap the input through a sine function multiple times
    // This creates multiple "harmonics" of the original signal
    float shaped = Math::sin(input * drive * M_PI * 0.5f);
    
    // Mix back with some of the original to keep the low-end "thump"
    return (shaped * 0.8f) + (input * 0.2f);
}

float applyWaveshape4(float input, float waveshapeAmount)
{
    float absX = fabsf(input);
    float amount = waveshapeAmount * 20.0f; // High gain scale
    
    // Nonlinear distortion: x / (1 + amount * |x|)
    // As 'amount' increases, this turns any wave into a hard square
    float dist = input * (1.0f + amount) / (1.0f + amount * absX);
    
    // To make it "fat," we add a DC-offset-style shift that we then clip
    // This emphasizes the sub-harmonics
    return dist * 0.9f; 
}

// apply waveshape using sinf when waveshapeAmount > 0, waveshape2 when waveshapeAmount < 0
float applyMultiWaveshape(float input, float waveshapeAmount)
{
    if (waveshapeAmount > 0.0f) {
        return applyWaveshape(input, waveshapeAmount);
    }
    if (waveshapeAmount < 0.0f) {
        return applyWaveshape2(input, -waveshapeAmount);
    }
    return input;
}
