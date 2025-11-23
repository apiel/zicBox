/** Description:
This header file defines a specialized, high-speed method for calculating the hyperbolic tangent function (known as `tanh`). This type of calculation is crucial in areas like digital signal processing, particularly in audio engineering for modeling effects such as distortion or saturation.

The primary goal of this code is speed. Standard mathematical functions can be computationally intensive, especially when needed thousands of times per second. To solve this, the function utilizes a "Lookup Table"—a pre-calculated list of `tanh` results stored directly in memory.

**How the Function Works:**

1.  **Input Safety:** The function first checks the input value. It ensures the number is safely restricted to a specific range (between -1.0 and 1.0), which prevents errors and focuses the processing on the most relevant part of the signal.
2.  **Indexing:** The restricted input value is then mathematically mapped to a specific index number. This index corresponds to an exact position within the pre-calculated lookup table.
3.  **Fast Retrieval:** Instead of performing a complex calculation, the function immediately retrieves and returns the stored result found at that calculated index.

By replacing real-time math with simple memory retrieval, this method provides extremely fast performance, making it highly effective for real-time applications like audio processing where latency is critical.

sha: e3ae553db27eeea3b5407b40920f8a783f157cb70c644b6a711d7506a42374d7 
*/
#pragma once

#include "helpers/clamp.h"
#include "audio/lookupTable.h"

float tanhLookup(float x, LookupTable* lookupTable)
{
    x = CLAMP(x, -1.0f, 1.0f);
    int index = static_cast<int>((x + 1.0f) * 0.5f * (lookupTable->size - 1));
    return lookupTable->tanh[index];
}