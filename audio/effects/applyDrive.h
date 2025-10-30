#pragma once

#include "./utils.h"

float applyDrive(float input, float driveAmount, LookupTable* lookupTable)
{
    if (driveAmount == 0.0f) {
        return input;
    }
    return tanhLookup(input * (1.0f + driveAmount * 5.0f), lookupTable);
}
