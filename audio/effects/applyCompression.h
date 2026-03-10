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
    float absInput = fabsf(input);
    if (absInput > 1.0f) absInput = 1.0f;
    float squash = absInput - (compressAmount * 0.333f * absInput * absInput * absInput);
    float output = (input > 0.0f) ? squash : -squash;
    // return output * (1.0f + compressAmount * 0.5f);
    return (output * compressAmount) + (input * (1.0f - compressAmount));
}

float applyCompression2(float input, float scaledIntensity, float& envelope)
{
    float curveIntensity = scaledIntensity * scaledIntensity;

    if (curveIntensity < 0.0001f) return input;

    float absInput = fabsf(input);
    envelope = (0.999f * envelope) + (0.001f * absInput);

    float threshold = 1.0f - (curveIntensity * 0.7f);
    float ratio = 1.0f + (curveIntensity * 4.0f);

    float output = input;

    if (envelope > threshold) {
        float compressedEnvelope = threshold + (envelope - threshold) / ratio;
        float gainReduction = compressedEnvelope / envelope;
        output *= gainReduction;
    }
    float makeup = 1.0f + (curveIntensity * 0.4f);
    return output * makeup;
}

// float applyCompression2(float input, float scaledIntensity, float &envelope)
// {
//     if (scaledIntensity <= 0.0f) return input;

//     float absInput = fabsf(input);
//     envelope = (0.999f * envelope) + (0.001f * absInput);

//     float threshold = 1.0f - (scaledIntensity * 0.8f); // Drops to 0.2
//     float ratio = 1.0f + (scaledIntensity * 8.0f);     // Up to 9:1 ratio

//     float output = input;

//     if (envelope > threshold) {
//         float compressedEnvelope = threshold + (envelope - threshold) / ratio;
//         float gainReduction = compressedEnvelope / envelope;
//         output *= gainReduction;
//     }

//     float makeupGain = 1.0f + (scaledIntensity * 1.5f);

//     return fminf(fmaxf(output * makeupGain, -1.0f), 1.0f);
// }
