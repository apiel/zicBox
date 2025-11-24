/** Description:
This code snippet defines a function that calculates an enhanced or "scaled" movement step, typically used for handling input from devices like digital encoders or dials. Its main purpose is to make the system more responsive to how quickly a user is manipulating the control.

### Purpose and Mechanism

The function requires two key pieces of information: the basic intended movement direction (e.g., positive or negative one step) and two timestamps that mark the time between the current movement step and the previous one.

The core idea is to measure the speed of the input:

1.  **Time Measurement:** The code first calculates the elapsed time between the two recorded steps. A very short duration indicates a fast rotation or movement.
2.  **Speed Factor:** This elapsed time is used to compute a "speed factor" or multiplier. If the user moves the dial quickly (short time difference), the speed factor becomes large. If the user moves slowly, the factor remains small.
3.  **Safety Limits:** To prevent erratic behavior or overreaction from an extremely fast input, the code limits this calculated speed factor. The scaling is safely capped between 1x (no speed acceleration) and 10x (maximum speed acceleration).

Finally, the original intended direction is multiplied by this adjusted speed factor. The result is a scaled movement value. This makes the input feel dynamic: a slow turn results in small, precise steps, while a fast turn results in large steps, allowing the user to traverse a range quickly.

sha: 64eeef851c0b13fb1a2c71ffdc50cc6deaa550af1b15a2de3673eebb3f3d04fd 
*/
#pragma once

#include <cmath>
#include <cstdint>

#include "helpers/clamp.h"

int encGetScaledDirection(int8_t direction, uint32_t tick, uint32_t lastTick)
{
    // Calculate time difference between ticks
    uint32_t delta = tick - lastTick;

    if (delta == 0)
        delta = 1;

    // Compute a speed factor: smaller delta = faster turn
    // Example: if delta = 200ms → factor = 1
    //          if delta = 10ms  → factor = 10
    // The constants can be tuned
    float speedFactor = 200.0f / delta;

    // Clamp to a reasonable range (1x to 10x)
    speedFactor = CLAMP(speedFactor, 1.0f, 10.0f);

    // Apply scaled direction (round to int)
    int scaledDirection = direction * (int)round(speedFactor);
    return scaledDirection;
}