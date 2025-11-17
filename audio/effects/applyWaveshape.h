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
