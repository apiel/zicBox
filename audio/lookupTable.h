/** Description:
This code defines a high-performance utility class called `LookupTable`. This tool is commonly used in applications that require extremely fast mathematical operations, such as audio synthesis, graphics rendering, or signal processing.

### Core Concept: Trading Space for Speed

The fundamental idea is optimization. Calculating complex mathematical functions repeatedly (like the sine wave) can be computationally expensive. Instead of recalculating them every time they are needed, the `LookupTable` pre-calculates thousands of results when the program starts and stores them in memory. When the application needs a result, it simply retrieves the saved value (looks it up) rather than performing the slow calculation. This retrieval is significantly faster than recalculation.

### What is Stored?

The table is defined to hold 8192 pre-calculated entries (`LOOKUP_TABLE_SIZE`). It contains three distinct lists (arrays) of numbers:

1.  **Sine Wave:** Values corresponding to a complete cycle of a sine wave, useful for generating oscillations or musical tones.
2.  **Hyperbolic Tangent (tanh):** Values for the hyperbolic tangent function, often used in complex signal shaping or non-linear processes (like neural networks).
3.  **Random Noise:** A long sequence of pre-generated random numbers.

### How it is Used

The class provides helper functions to quickly access this stored data:

*   **Retrieving Sine Values:** Functions allow the user to input a "phase" (a position within the wave cycle), and the class quickly returns the corresponding sine value stored in the table.
*   **Retrieving Noise:** A dedicated function retrieves the next sequential random number from the stored list. If it reaches the end of the 8192 numbers, it seamlessly wraps back to the beginning to start retrieving the sequence again.

sha: e7941b1253259e2739a4e89b0528d45cadffabc6582b3e5f90f9476df6dfce54 
*/
#pragma once

#include <math.h>

#include "audio/utils/noise.h"

// #define LOOKUP_TABLE_SIZE 4096
#define LOOKUP_TABLE_SIZE 8192
class LookupTable {
public:
    const static int size = LOOKUP_TABLE_SIZE;
    float sine[size];
    float tanh[size];

    LookupTable()
    {
        for (int i = 0; i < size; i++) {
            float normalizedX = (float)i / (float)(size - 1) * 2.0f - 1.0f; // Map index to [-1.0, 1.0]
            sine[i] = sinf(normalizedX * M_PI);
            tanh[i] = std::tanh(normalizedX); // Precompute tanh
        }
    }

    float getNoise()
    {
        return makeNoise();
    }

    float getSin(float phase)
    {
        int idx = (int)(phase * size) % size;
        if (idx < 0)
            idx += size;
        return sine[idx];
    }

    float getSin2(float phase) {
        // phase expected in [0, 1)
        phase = fmodf(phase, 1.0f);
        if (phase < 0) phase += 1.0f;
        int idx = (int)(phase * (size - 1));
        return sine[idx];
    }
};
