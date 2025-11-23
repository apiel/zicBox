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

#include "audio/lookupTable.h"
#include "audio/utils/linearInterpolation.h"

// apply waveshape using lookup table
float applyWaveshapeLut(float input, float waveshapeAmount, LookupTable* lookupTable)
{
    float x = input -std::floor(input);
    float sineValue = linearInterpolation(x, lookupTable->size, lookupTable->sine);
    return input + waveshapeAmount * sineValue * 2;
}

// apply waveshape using sinf
float applyWaveshape(float input, float waveshapeAmount)
{
    float sineValue = sinf(input);
    return input + waveshapeAmount * sineValue * 2;
}

// apply waveshape using sinf when waveshapeAmount > 0, look table when waveshapeAmount < 0
float applyWaveshape(float input, float waveshapeAmount, LookupTable* lookupTable)
{
    if (waveshapeAmount > 0.0f) {
        return applyWaveshape(input, waveshapeAmount);
    }
    if (waveshapeAmount < 0.0f) {
        return applyWaveshapeLut(input, -waveshapeAmount, lookupTable);
    }
    return input;
}
