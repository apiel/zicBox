/** Description:
This code defines a specialized utility function, typically used in real-time signal processing or audio applications, known as a **decimator** or downsampler.

### Core Purpose

The primary goal of this function is to intentionally reduce the effective rate at which a continuous stream of data (like an audio signal) is processed. It simulates lowering the sampling rate, which in digital effects creates a "stepped" or gritty sound by discarding most of the incoming information.

### Mechanism

The function operates using a controlled skipping mechanism. It takes a new input value, an adjustment setting (the `amount`), and relies on two persistent variables: a held output value and a running counter.

1.  **Setting the Skip Rate:** The function first calculates the necessary "skip interval" based on the `amount` provided. A higher adjustment amount results in a larger interval, meaning more inputs will be ignored.
2.  **Filtering Input:** It checks the running counter against this calculated interval.
3.  **Output Hold:** If the counter does not match the skip interval, the function ignores the new input and returns the previously saved output value.
4.  **Update:** Only when the counter successfully hits the required interval does the function accept the current input, save it as the new output, and then return this updated value.

By consistently holding the last accepted value for several cycles, the function ensures the output signal changes less frequently than the input, resulting in a distinct, discontinuous effect.

sha: 47f14af15279f96cd447e57baa44e25cfbcb7a8628d9b0800575c70f578c2b9f 
*/
#pragma once

float applyDecimator(float input, float amount, float& decimHold, int& decimCounter)
{
    int interval = 1 + (int)(amount * 30.0f); // Downsample ratio
    if (decimCounter % interval == 0) {
        decimHold = input;
    }
    decimCounter++;
    return decimHold;
}
