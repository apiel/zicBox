/** Description:
This file serves as a blueprint for a specialized utility tool designed to generate pseudo-random numbers. It packages standard randomization features into an easy-to-use structure named "Random."

### Purpose and Mechanism

The primary goal of this utility is to produce sequences of numbers that appear random. It achieves this by relying on two main components:

1.  **Seeding:** To start the random process, the utility uses the computer's current system time. This initial value, called the "seed," determines the sequence of random numbers that follows.
2.  **Improving Randomness:** A common issue is calling a random function repeatedly in quick succession (like within the same millisecond), which often results in the same number. To mitigate this, the utility adds a small internal counter to the system time every time a random number is requested, ensuring the seed changes slightly on each call.

### Key Utilities

The tool provides distinct ways to access the generated randomness:

*   **`get()`:** Provides a raw, large random integer (a whole number).
*   **`toPct()`:** Converts any generated random integer into a floating-point percentage value, resulting in a number between 0.0 and 1.0. This is useful for probability calculations.
*   **`pct()`:** Offers the most convenient function, directly generating a random number as a percentage (between 0.0 and 1.0) in a single step.

sha: 267361fb69988b1ee99c936d735ae4459ff3d62e5d03cf5f62111ac9b09ce0ea 
*/
#pragma once

#include <stdlib.h>
#include <time.h>
#include <cstdint>

class Random {
protected:
    int randCounter = 0;

public:
    int get()
    {
        // could create a lookup table ?
        srand(time(NULL) + randCounter++);
        return rand();
    }

    float pct()
    {
        return toPct(get());
    }

    float toPct(int value)
    {
        return value / (float)RAND_MAX;
    }
};

static uint32_t hash32(uint32_t x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

static float rand01(uint32_t& seed)
{
    seed = hash32(seed);
    return (seed & 0xFFFFFF) / float(0xFFFFFF);
}