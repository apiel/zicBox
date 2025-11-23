/** Description:
This C++ structure, named `AsrEnvelop`, acts as a sophisticated timing controller, often referred to as an "envelope generator." It is commonly used in audio synthesis (like determining a note’s volume profile) or other systems that require smooth transitions between zero and maximum values.

### Core Functionality

The class models the standard Attack-Sustain-Release (ASR) phases:

1.  **Attack:** The internal value rises rapidly from zero to its maximum (1.0).
2.  **Sustain:** The value holds steady at the maximum.
3.  **Release:** The value falls smoothly back to zero.

### How the Code Works

The core of the system is a simple state machine. The class holds an internal pointer that always points to the specific function responsible for the current phase (Attack, Sustain, or Release).

*   **Driving Time:** The main public method, `next()`, is called repeatedly (usually synchronized with system time). When called, it executes the function currently pointed to, calculating and returning the next incremental output value.
*   **Transitioning Stages:** Methods like `attack()` and `release()` manually switch the internal pointer to the desired phase, initiating the corresponding rise or fall in the output value.
*   **Customization:** The speed of the Attack (rise) and Release (fall) phases are customizable and defined when the object is initialized.
*   **Completion:** Once the Release phase finishes and the value reaches zero, the class executes an optional user-defined function (a "callback") to signal that the profile is complete.

sha: 0c8d2263ff83b561d6ba0fdd0a2c98598b9cdb6b7903205cb3a4335c4830bb54 
*/
#pragma once

#include <functional>

class AsrEnvelop {
protected:
    float (AsrEnvelop::*nextFn)() = &AsrEnvelop::envelopSustain;

    float env = 0.0f;
    float* attackStep;
    float* releaseStep;

    std::function<void()> onFinish;

    float envelopAttack()
    {
        env += *attackStep;
        if (env >= 1.0f) {
            env = 1.0f;
            nextFn = &AsrEnvelop::envelopSustain;
        }
        return env;
    }

    float envelopSustain()
    {
        return env;
    }

    float envelopRelease()
    {
        env -= *releaseStep;
        if (env <= 0.0f) {
            env = 0.0f;
            if (onFinish) {
                onFinish();
            }
        }
        return env;
    }

public:
    AsrEnvelop(float* attackStep, float* releaseStep, std::function<void()> onFinish = NULL)
        : attackStep(attackStep)
        , releaseStep(releaseStep)
        , onFinish(onFinish)
    {
    }

    float next()
    {
        return (this->*nextFn)();
    }

    float get()
    {
        return env;
    }

    void attack()
    {
        nextFn = &AsrEnvelop::envelopAttack;
    }

    void release()
    {
        nextFn = &AsrEnvelop::envelopRelease;
    }

    bool isRelease()
    {
        return nextFn == &AsrEnvelop::envelopRelease;
    }
};
