#pragma once

#include "./utils.h"

float applySoftClipping(float input, LookupTable* lookupTable)
{
    return tanhLookup(input, lookupTable);
}
