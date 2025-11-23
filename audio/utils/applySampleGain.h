/** Description:
**Analysis of the Data Scaling Utility**

This code defines a specialized utility function commonly used in signal processing, such as digital audio or sensor data analysis. It is designed to be easily included in other program files via a header, simplifying its reuse across an application.

**Core Purpose (Normalization)**

The primary goal of this function is to perform a technique called *normalization*. Normalization scales an entire set of numerical inputs so that the loudest or largest data point reaches a specific maximum value (often 1.0), ensuring the data utilizes the full available dynamic range without distortion.

**How the Function Works**

The process involves two main stages to ensure maximum efficiency:

1.  **Finding the Peak:** The function first scans the entire provided list of numbers. It meticulously searches for the largest value, ignoring whether that number is positive or negative (finding the absolute maximum, or "peak volume").
2.  **Applying the Gain:** Once the maximum peak is identified, the function calculates a precise scaling factor, often referred to as "gain." It then revisits every single number in the original list and multiplies it by this calculated gain.

**Result and Benefit**

The outcome is a modified list where the signal’s intensity has been maximized. This scaling ensures that the data is as robust as possible without "clipping" or exceeding system limits. While the overall intensity is increased, the fundamental shape and relative proportions of the data remain perfectly preserved.

sha: 2d790f818497f300e1ec359765e267a24c6d5929cb46b0f49402d8a63a1074aa 
*/
#pragma once

#include <cstdint>

void applySampleGain(float* buffer, uint64_t count)
{
    float max = 0.0;
    for (uint64_t i = 0; i < count; i++) {
        if (buffer[i] > max) {
            max = buffer[i];
        } else if (-buffer[i] > max) {
            max = -buffer[i];
        }
    }
    float gain = 1.0 / max;
    // printf("max sample: %f gain: %f\n", max, gain);
    for (uint64_t i = 0; i < count; i++) {
        buffer[i] = buffer[i] * gain;
    }
}
