/** Description:
This file defines a highly specialized function used for real-time signal processing, often in audio applications like music software or effect pedals.

**Purpose: Soft Clipping**
The central goal of the function is to perform "soft clipping." This is a technique used to smoothly limit the maximum volume or amplitude of a signal. Unlike harsh digital clipping (which abruptly chops off the signal peaks), soft clipping applies a gentle compression, preventing the signal from exceeding a certain level while introducing a smooth, pleasant distortion characteristic—a desirable effect in creating overdrive or vintage audio sounds.

**How the Code Works**
The smooth limiting effect is achieved using mathematics related to the hyperbolic tangent function. To make this process fast enough for real-time use, the system avoids calculating the complex math on the fly. Instead, it relies on an optimization technique: a "lookup table."

The function `applySoftClipping` takes the input signal and a reference to this pre-calculated table. It immediately calls a helper function that quickly finds the corresponding limited output value within the table. This use of pre-calculated data makes the entire clipping process incredibly fast and efficient.

In essence, this file provides a crucial, optimized routine for creating analog-like volume limiting and distortion effects.

sha: dee74fe29ae4f10233e06ac5cc6cffa9174a7fcd347bfd2a158ebd7b3ee37cbf 
*/
#pragma once

#include "./utils.h"

float applySoftClipping(float input, LookupTable* lookupTable)
{
    return tanhLookup(input, lookupTable);
}
