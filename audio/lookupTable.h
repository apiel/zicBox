#pragma once

#include <math.h>

// #define LOOKUP_TABLE_SIZE 4096
#define LOOKUP_TABLE_SIZE 8192
// #define LOOKUP_TABLE_SIZE 16384

class LookupTable {
protected:
    int noiseIndex = 0;

public:
    const static int size = LOOKUP_TABLE_SIZE;
    float sine[size];
    float tanh[size];
    float noise[size];

    LookupTable()
    {
        for (int i = 0; i < size; i++) {
            float normalizedX = (float)i / (float)(size - 1) * 2.0f - 1.0f; // Map index to [-1.0, 1.0]
            sine[i] = sinf(normalizedX * M_PI);
            tanh[i] = std::tanh(normalizedX); // Precompute tanh
            noise[i] = 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f; // Random noise [-1.0, 1.0]
        }
    }

    float getNoise()
    {
        if (noiseIndex >= size) {
            noiseIndex = 0;
        }
        return noise[noiseIndex++];
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
