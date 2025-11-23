/** Description:
This header file serves as a blueprint defining a single, specialized mathematical operation designed to safely modify and regulate numerical values. This type of functionality is often critical in fields like audio processing, computer graphics, or physics simulations where signals must be strictly controlled.

### Purpose and Mechanism

The function defined here, `applyClipping`, takes a primary input value and a separate "scaling" factor. Its primary objective is to boost or scale the input value while ensuring the final result remains within safe, predefined limits.

**How It Works (The Basic Idea):**

1.  **Bypass Check:** If the provided scaling factor is zero, the function recognizes that no modification is necessary and returns the original input value immediately.
2.  **Scaling and Boosting:** If the scaling factor is non-zero, the input value is amplified or adjusted. This process effectively boosts the magnitude of the signal based on the factor provided.
3.  **Safety Limit (Clamping):** Crucially, after the input is boosted, the resulting value is subjected to an external safety measure (a helper utility typically called `CLAMP`). This safety measure forces the final output to stay within a specific numerical range. In this case, the result is guaranteed never to exceed a maximum of 1.0 and never fall below a minimum of -1.0. This prevents the signal from becoming too large or "clipping" improperly outside of acceptable boundaries.

This setup ensures that even if heavy scaling is applied, the output remains safe and consistent for subsequent processes.

sha: 9e6e4daa0452774df195eb622b80153f56fbfa3b65764163dc2db5b82e49b5b2 
*/
#pragma once

#include "helpers/clamp.h"

float applyClipping(float input, float scaledClipping)
{
    if (scaledClipping == 0.0f) {
        return input;
    }
    return CLAMP(input + input * scaledClipping, -1.0f, 1.0f);
}
