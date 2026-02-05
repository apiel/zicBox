/** Description:
This piece of code defines a specialized function designed to implement an audio effect known as "drive" or "saturation." This effect is commonly used to add warmth, grit, or distortion to a signal, similar to overdriving an analog amplifier.

### Purpose and Mechanism

The function accepts three inputs: the original signal value, the desired intensity of the effect (`driveAmount`), and a pre-calculated data structure called a `LookupTable`.

1.  **Bypass Check:** First, the function checks if the requested `driveAmount` is zero. If it is, the original signal is returned immediately without any processing, ensuring efficient operation when the effect is turned off.
2.  **Signal Amplification:** If the drive is active, the original signal is greatly amplified based on the `driveAmount`. A higher drive setting leads to a much stronger signal boost.
3.  **Non-linear Shaping:** The boosted, loud signal is then passed through a mathematical shaping process. This process uses a function often referred to as hyperbolic tangent (`tanh`), which is essential for creating musical-sounding, smooth distortion (soft clipping).
4.  **Optimization:** To ensure this complex calculation runs quickly enough for real-time tasks (like playing music without delay), the function relies on the provided `LookupTable`. This table acts as a quick cheat sheet, allowing the system to instantly retrieve the necessary shaped value instead of performing a time-consuming mathematical computation every time.

In summary, the code boosts the input signal and then uses a highly optimized method to "softly clip" the peaks, resulting in the characteristic warmth and distortion associated with analog drive effects.

sha: 7e53d887dd5dd880f83ee7b9ec12753f1777dcff059f5f160d0bcb44b6018212
*/
#pragma once

#include "audio/lut/getTanh.h"

float applyDrive(float input, float driveAmount)
{
    if (driveAmount == 0.0f) {
        return input;
    }
    return getTanh(input * (1.0f + driveAmount * 5.0f));
}
