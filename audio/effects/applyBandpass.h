/** Description:
This code defines a core mathematical utility used in digital audio processing: a **Bandpass Filter**.

In simple terms, a filter acts like a selective acoustic gate. This particular filter allows only a specific range of frequencies to pass through, effectively blocking both very high (treble) and very low (bass) frequencies outside that range.

The function operates on a single audio sample at a time. It requires several key inputs: the current sound value, the overall recording speed (sample rate), the desired central frequency it should focus on, and a "Quality Factor" (`Q`), which determines how wide or narrow the accepted frequency range is.

The basic idea relies on precise mathematical calculations that determine filter coefficients, essentially setting up the rules for how the sound should be modified. Crucially, digital filters must have memory. The function stores past input and output values, which are immediately incorporated into the next calculation. This ensures smooth, continuous filtering rather than treating each sound sample in isolation.

The function takes the current sample, combines it with the memory of the last few filtered results, applies the rigorous filtering math, and produces one filtered output sample. When this process is repeated rapidly (thousands of times per second), it transforms the entire audio stream according to the desired center frequency.

sha: 79a4fe787c6469610a4192dc0bb5b8918c45a83f6ab9d34bcc9dbf79cbc47ba2 
*/
#pragma once

#include <cmath>

float applyBandpass(float x, float center, float Q, int sampleRate, float& bp_x1, float& bp_x2, float& bp_y1, float& bp_y2)
{
    float omega = 2.f * M_PI * center / sampleRate;
    float alpha = sinf(omega) / (2.f * Q);

    float b0 = alpha;
    float b1 = 0.f;
    float b2 = -alpha;
    float a0 = 1.f + alpha;
    float a1 = -2.f * cosf(omega);
    float a2 = 1.f - alpha;

    float y = (b0 / a0) * x + (b1 / a0) * bp_x1 + (b2 / a0) * bp_x2
        - (a1 / a0) * bp_y1 - (a2 / a0) * bp_y2;

    bp_x2 = bp_x1;
    bp_x1 = x;
    bp_y2 = bp_y1;
    bp_y1 = y;

    return y;
}