/** Description:
This C++ header defines an optimized mathematical function used in Digital Signal Processing (DSP), typically for real-time audio applications like music synthesizers or effects units.

### Function Purpose: Bandpass Filter

The function's main job is to apply a specialized audio effect known as a **Bandpass Filter**. This filter allows sound frequencies within a narrow, defined range (the "band") to pass through, while aggressively suppressing or muting sounds that are either too high or too low. It is often used to isolate specific tones or sculpt sound texture.

### How the Code Works (The Basic Idea)

The code executes a classic digital filter calculation, optimized for speed:

1.  **Setting Parameters:** The function first takes key inputs: the desired *center frequency*, the *quality factor* (Q, which determines how sharp or wide the filter’s effect is), and the audio stream’s *sample rate*.
2.  **Calculating Coefficients:** Using optimized trigonometric functions (like fast Sine and Cosine), it translates these user settings into a set of mathematical constants (coefficients) required for the filter equation.
3.  **Applying the Filter:** It takes one piece of incoming sound data (a "sample") and feeds it into the filter equation. Crucially, digital filters require internal memory. The function uses dedicated variables to store the values of the two most recent inputs and two most recent outputs, which are combined with the current input to calculate the final, filtered output.
4.  **Updating Memory:** After the new output is calculated, the memory variables are updated, preparing the filter to smoothly process the very next incoming sound sample.

The term "Fast" in the function name highlights that it prioritizes computational efficiency, essential for ensuring audio processing occurs without audible delay.

sha: 27d54bffa256c99c69e2c6d0ffbb682847bc5cb657608b99b3c6fbff85a4f787 
*/
#pragma once

#include "helpers/math.h"

float applyBandpassFast(float x, float center, float Q, int sampleRate, float& bp_x1, float& bp_x2, float& bp_y1, float& bp_y2)
{
    float omega = 2.f * M_PI * center / sampleRate;
    float alpha = fastSin(omega) / (2.f * Q);

    float b0 = alpha;
    float b1 = 0.f;
    float b2 = -alpha;
    float a0 = 1.f + alpha;
    float a1 = -2.f * fastCos3(omega);
    float a2 = 1.f - alpha;

    float y = (b0 / a0) * x + (b1 / a0) * bp_x1 + (b2 / a0) * bp_x2
        - (a1 / a0) * bp_y1 - (a2 / a0) * bp_y2;

    bp_x2 = bp_x1;
    bp_x1 = x;
    bp_y2 = bp_y1;
    bp_y1 = y;

    return y;
}