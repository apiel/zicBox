/** Description:
## C/C++ Header Function Analysis: Audio Bitcrusher

This header function, `applyBitcrusher`, is designed for digital audio processing. Its sole purpose is to simulate a "bitcrusher" effect, which intentionally degrades the quality of a sound signal to create a characteristic vintage, harsh, or robotic distortion.

### 1. Function Goal

The function processes a single incoming audio sample (`input`) and applies two forms of digital degradation based on the desired `amount` (where 0 is no effect and 1 is maximum effect).

### 2. Core Mechanism

The bitcrusher effect is achieved by combining two distinct forms of data reduction:

**A. Bit Depth Reduction (Quantization)**
The function first limits the precision of the audio signal. Normal digital audio uses many levels to represent volume changes smoothly. By calculating a reduced "bit depth," the function forces the sample to snap to a limited number of defined volume steps. This process, called quantization, creates a noticeable digital harshness or noise.

**B. Sample Rate Reduction (Sample & Hold)**
This is responsible for the effect’s distinct "stepped" sound. The function uses an internal counter to track time. Instead of updating the sound sample immediately after calculating the crushed value, it only updates after a certain number of cycles, determined by the `amount`. In the intervening cycles, the function returns the last calculated degraded value (the "sample hold"). This lack of update creates the auditory illusion of a much lower sample rate, resulting in a stuttering, broken texture.

### 3. Control and Mixing

The function is highly controllable:

*   If the `amount` is zero, the original sound passes through untouched.
*   The overall `amount` controls the severity of both the bit depth reduction and the stepping frequency.
*   For very subtle effects, the function includes a blending stage, mixing the slightly degraded output with the original clean signal to ensure a smooth transition.

This function relies on external tracking variables (the hold value and the sample counter) to maintain the continuous state and rhythm of the distortion across an entire stream of audio data.

sha: 3bafa9f4e473886b1c5e6547b879fa2a7a21146fe7e4aea1fb4db4428ce8a21b 
*/
#pragma once

#include <cmath>

float applyBitcrusher(float input, float amount, float& sampleHold, int& sampleCounter)
{
    if (amount == 0.0f) {
        return input;
    }

    // Reduce Bit Depth
    int bitDepth = 2 + amount * 10; // Stronger effect
    float step = 1.0f / (1 << bitDepth); // Quantization step
    float crushed = round(input / step) * step; // Apply bit reduction

    // Reduce Sample Rate
    int sampleRateDivider = 1 + amount * 20; // Reduces update rate
    if (sampleCounter % sampleRateDivider == 0) {
        sampleHold = crushed; // Hold the value for "stepping" effect
    }
    sampleCounter++;

    if (amount < 0.1f) {
        // mix with original signal
        return sampleHold * (amount * 10) + input * (1.0f - (amount * 10));
    }

    return sampleHold;
}