/** Description:
This code defines a specialized tool, likely used within a larger system like an audio engine or a real-time signal processor. Its primary function is to implement a mechanism known as "sample reduction" or "data decimation."

### Overview and Purpose

The core job of this function is to intentionally reduce the effective rate at which a stream of continuous data (like an audio waveform) is updated. Instead of processing every incoming data point, it holds onto the last accepted value for a specific duration, creating a stepped or blocky effect on the signal.

### How It Works

1.  **Control:** The function takes a new incoming data value and an `amount` setting, acting as a control knob. If this `amount` is set to zero, the function is bypassed, and the data passes straight through without modification.
2.  **Counting Time:** If the reduction is active, the function uses an internal counter to track how many incoming data points have arrived since the last update.
3.  **Hold State:** While the counter is running and has not reached its limit (a threshold determined by the `amount` setting), the function ignores the fresh input data. It repeatedly outputs the value it accepted during the previous cycle. This creates the "stepped" effect.
4.  **Update:** Once the counter hits the preset threshold, it accepts the brand new input value, resets the counter to zero, and that new value becomes the held output for the next cycle.

In essence, this function acts like a selective filter, slowing down the frequency of data updates based on the user-defined `amount`.

sha: 1cd564a8873feffc0edf41366b2501f2fd1858d8bc8be39e047e6858b4b5712b 
*/
#pragma once

float applySampleReducer(float input, float amount, float& sampleSqueeze, int& samplePosition)
{
    if (amount == 0.0f) {
        return input;
    }
    if (samplePosition < amount * 100 * 2) {
        samplePosition++;
    } else {
        samplePosition = 0;
        sampleSqueeze = input;
    }

    return sampleSqueeze;
}