#pragma once

#include <math.h>

#include "helpers/range.h"
#include "plugins/audio/utils/lookupTable.h"
#include "plugins/audio/utils/utils.h"

float tanhLookup(float x, LookupTable* lookupTable)
{
    x = range(x, -1.0f, 1.0f);
    int index = static_cast<int>((x + 1.0f) * 0.5f * (lookupTable->size - 1));
    return lookupTable->tanh[index];
}

float sineLookupInterpolated(float x, LookupTable* lookupTable)
{
    x -= std::floor(x);
    return linearInterpolation(x, lookupTable->size, lookupTable->sine);
}

float applySoftClipping(float input, LookupTable* lookupTable)
{
    return tanhLookup(input, lookupTable);
}

float applyWaveshape(float input, float waveshapeAmount, LookupTable* lookupTable)
{
    if (waveshapeAmount > 0.0f) {
        // float sineValue = sineLookupInterpolated(input);
        float sineValue = sinf(input);
        return input + waveshapeAmount * sineValue * 2;
    }
    if (waveshapeAmount < 0.0f) {
        float sineValue = sineLookupInterpolated(input, lookupTable);
        return input + (-waveshapeAmount) * sineValue;
    }
    return input;
}

float applyBoost(float input, float bassBoostAmount, float& prevInput, float& prevOutput)
{
    if (bassBoostAmount == 0.0f) {
        return input;
    }
    float bassFreq = 0.2f + 0.8f * bassBoostAmount;
    float bassBoosted = (1.0f - bassFreq) * prevOutput + bassFreq * (input + prevInput) * 0.5f;
    prevInput = input;
    prevOutput = bassBoosted;
    bassBoosted *= 1.0f + bassBoostAmount * 2.0f;

    return bassBoosted;
}

float applyDrive(float input, float driveAmount, LookupTable* lookupTable)
{
    if (driveAmount == 0.0f) {
        return input;
    }
    return tanhLookup(input * (1.0f + driveAmount * 5.0f), lookupTable);
}

float applyCompression(float input, float compressAmount)
{
    if (compressAmount == 0.0f) {
        return input;
    }
    if (input > 0.0f) {
        return std::pow(input, 1.0f - compressAmount * 0.8f);
    }
    return -std::pow(-input, 1.0f - compressAmount * 0.8f);
}

float applyClipping(float input, float scaledClipping)
{
    if (scaledClipping == 0.0f) {
        return input;
    }
    return range(input + input * scaledClipping, -1.0f, 1.0f);
}

float applyHighFreqBoost(float input, double& boostTimeInc, double& boostTime)
{
    if (boostTimeInc == 0) {
        return input;
    }
    // Simple high-shelf boost logic
    // TODO optimize precalculate: highBoost.get() * time
    // float highFreqComponent = input * (highBoost.get() * boostTime); // Emphasize high frequencies
    float highFreqComponent = input * boostTime; // Emphasize high frequencies
    boostTime += boostTimeInc;
    return input + highFreqComponent;
}
