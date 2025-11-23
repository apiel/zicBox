/** Description:
This file serves as a standardized blueprint, or "interface," for managing digital sound waves (known as wavetables) within an audio processing program, such as a synthesizer or signal analyzer.

**Basic Idea of Functionality:**

This code defines a set of required rules that any specific type of sound wave—like a sine wave, a square wave, or a complex custom wave—must adhere to. It doesn't contain the actual logic for generating the sound; rather, it dictates the required structure for any class that *will* generate it.

**Key Components:**

1.  **Sound Length:** It holds a mandatory variable called `sampleCount`, which records the exact length (number of data points) of the sound wave being managed.
2.  **Mandatory Rules:** The core of this blueprint is a set of functions that are marked as required. Any component built using this blueprint must implement these functions:
    *   **Retrieve Specific Samples:** Functions designed to calculate and return individual sound data points. These might take into account the current position in the wave or the desired frequency (pitch) to determine the output.
    *   **Retrieve All Samples:** Functions to provide the entire set of sound wave data points at once, usually for playback or display.

**Why this structure is useful:**

By setting up this strict interface, the program ensures consistency. Regardless of whether a specific sound generator creates a simple tone or a complex texture, the rest of the audio engine knows exactly how to ask for the data, promoting compatibility and easy swapping of different sound types.

sha: 8ada600033a4982053c19377826d6ad12e5a532029ee18fe2b376a391717f6fe 
*/
#pragma once

#include <cstdint>

class WavetableInterface {
public:
    uint64_t sampleCount;

    WavetableInterface(uint64_t sampleCount)
        : sampleCount(sampleCount)
    {
    }

    virtual float sample(float* index, float freq) = 0;
    virtual float* sample(float* index) = 0;
    virtual float* samples() = 0;
};
