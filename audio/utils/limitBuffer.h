/** Description:
This code defines a utility function primarily used for signal processing, often applied to digital audio or graphical data. Its core purpose is "limiting" or volume control, ensuring that a set of numerical data points does not exceed a predefined maximum amplitude.

**How it works:**

1.  **Peak Detection:** The function first analyzes the entire dataset (buffer). It scans every single value to find the number that is furthest away from zero—this represents the loudest or largest "peak," regardless of whether the value is highly positive or highly negative.
2.  **Limit Check:** This identified maximum peak value is then compared against a specified maximum allowed boundary (the "limit").
3.  **Proportional Scaling:** If the detected peak is greater than the allowed limit, the entire signal is deemed too loud or too large. The code calculates a precise scaling factor (a ratio) needed to reduce the peak down to the allowed limit.
4.  **Application:** This scaling factor is then applied uniformly to *every* single data point in the buffer. This action effectively turns down the volume of the entire signal proportionally, guaranteeing that the new maximum peak exactly matches the specified limit, preventing data overflow or distortion.

sha: c29790f739f4e2f4c01c95cb0303d6de6a0fb486ab3b86ebd80e870bc5eb50e0 
*/
#pragma once

#include <cstdint>

void limitBuffer(float* buffer, uint64_t count, float limit)
{
    float max = 0.0;
    for (uint64_t i = 0; i < count; i++) {
        if (buffer[i] > max) {
            max = buffer[i];
        } else if (-buffer[i] > max) {
            max = -buffer[i];
        }
    }

    if (max > limit) {
        float ratio = limit / max;
        for (uint64_t i = 0; i < count; i++) {
            buffer[i] = buffer[i] * ratio;
        }
    }
}
