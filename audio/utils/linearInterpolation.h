/** Description:
This header file defines essential mathematical utilities for smoothly estimating data points that fall between known values stored in a fixed list (often called a Look-Up Table, or LUT).

The primary technique employed is **Linear Interpolation**. This process helps estimate a new value by drawing a straight line between two adjacent existing data points. If you request a position that is, for example, 75% of the way between value A and value B, the system calculates a weighted average: 25% of A plus 75% of B.

The core functionality is provided by two functions:

1.  **`linearInterpolationAbsolute`**: This function is the engine that performs the actual blending. It takes a detailed position (which might include fractions, like 5.5) and retrieves the two surrounding known data points from the list. It then uses the fractional part of the position (0.5 in this example) to calculate the precise intermediate value. Crucially, it is designed to handle "wraparound," meaning if you request a position slightly beyond the end of the list, it intelligently wraps back to the beginning.

2.  **`linearInterpolation`**: This is a convenience function built on top of the first. Instead of requiring a specific position number, it lets users provide a relative index between 0.0 and 1.0 (representing 0% to 100% across the entire list). This simplifies usage by automatically converting that percentage into the necessary absolute position before the core interpolation calculation is performed.

In essence, this code ensures that accessing data across a predefined list results in smooth, continuous transitions rather than abrupt steps between stored values.

sha: 3b02b25051c8c74b6d90ac5d5475b4bf246201bf2b80a2fc794eb7f7c956490b 
*/
#pragma once

#include <cstdint>

float linearInterpolationAbsolute(float index, uint16_t lutSize, float* lut)
{
    uint16_t index1 = static_cast<uint16_t>(index);
    uint16_t index2 = (index1 + 1) % lutSize;
    float fractional = index - index1;

    // Interpolate between the two LUT values
    return lut[index1] * (1.0f - fractional) + lut[index2] * fractional;
}

// Linear interpolation relative where index is between 0.0f and 1.0f
float linearInterpolation(float index, uint16_t lutSize, float* lut)
{
    return linearInterpolationAbsolute(index * (lutSize - 1), lutSize, lut);
}
