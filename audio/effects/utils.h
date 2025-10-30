#pragma once

#include "helpers/clamp.h"
#include "audio/lookupTable.h"

float tanhLookup(float x, LookupTable* lookupTable)
{
    x = CLAMP(x, -1.0f, 1.0f);
    int index = static_cast<int>((x + 1.0f) * 0.5f * (lookupTable->size - 1));
    return lookupTable->tanh[index];
}