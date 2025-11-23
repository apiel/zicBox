/** Description:
This header defines a single utility function designed for **dynamic range compression** or signal limiting, a technique often used in audio processing or graphics to manage extreme values.

### Core Purpose

The function, `applyCompression`, takes a numeric input (like a sound level or signal intensity) and an adjustable `compressAmount`. Its main job is to "tame" or scale down the input value, particularly when the input is large, to prevent distortion or clipping.

### How It Works

1.  **Bypass Check:** If the requested `compressAmount` is zero, the function does nothing and immediately returns the original input value.
2.  **Mathematical Scaling:** If compression is applied, the function uses a powerful mathematical calculation (based on exponents) to modify the input. The higher the `compressAmount`, the greater the dampening effect on the input signal.
3.  **Maintaining Direction:** It is carefully designed to handle both positive and negative input values correctly. If the original input was positive, the result remains positive; if the original input was negative, the result remains negative, ensuring the signal's shape or polarity is preserved during compression.

In essence, this code acts as a customizable "soft limiter," reducing the overall loudness or magnitude of a signal in a smooth, non-linear way, based on the compression setting provided.

sha: 923e3ec6b3856a1fca30b4fda9e3594523d2c293b6a32b2e540cb72cb93e4435 
*/
#pragma once

#include <math.h>

float applyCompression(float input, float compressAmount)
{
    if (compressAmount == 0.0f) {
        return input;
    }
    // How about?
    // return (input * (1 - compressAmount)) + (range(std::pow(input, compressAmount * 0.8f), -1.0f, 1.0f) * fxAmount.pct());
    if (input > 0.0f) {
        return std::pow(input, 1.0f - compressAmount * 0.8f);
    }
    return -std::pow(-input, 1.0f - compressAmount * 0.8f);
}
