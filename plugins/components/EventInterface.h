/** Description:
This C++ header file defines a crucial system blueprint named the `EventInterface`. Its primary purpose is to establish a required contract for any module within the application that needs to respond to user input or system changes.

This interface acts purely as a template or "contract." Any actual event handler component in the system must implement all the functions defined here, ensuring a standardized way to process actions.

The contract requires handling several types of user interactions:

1.  **Motion Control:** Specific functions are required to process continuous physical interaction, such as swiping or dragging, and when that movement stops. This relies on a separate Motion component for detailed input.
2.  **Rotary Input:** The interface manages input from physical dials or knobs, often called "encoders." It records which dial was turned, the direction of rotation, and the precise time the action occurred.
3.  **Key Input:** It handles button presses or key strokes, identifying which key was used and its current state (pressed or released).
4.  **System Events:** A function is dedicated to managing layout changes, ensuring the system can properly adjust its visuals when resized.

The interface also defines a dedicated structure used to store the screen location and properties of the rotary dials. A required query function allows the system to efficiently determine which dial corresponds to a specific coordinate on the screen.

In essence, this blueprint ensures comprehensive and structured management of all user interaction and system responsiveness within the application.

sha: 9ccd293967cb32fdcae3bb9d2482fb525b74b47455f10e0d00b3d19c6f15a889 
*/
#pragma once

#include <cstdint>
#include <vector>

#include "plugins/components/motionInterface.h"

class EventInterface {
public:
    virtual void onMotion(MotionInterface& motion) = 0;
    virtual void onMotionRelease(MotionInterface& motion) = 0;
    virtual void onEncoder(int8_t id, int8_t direction, uint64_t tick) = 0;
    virtual void onKey(uint16_t id, int key, int8_t state) = 0;
    virtual void resize() = 0;

    struct EncoderPosition {
        int8_t id;
        Size size;
        Point pos;
        bool allowMotionScroll = true;
    };
    virtual int8_t getEncoderId(int32_t x, int32_t y, bool isMotion = false) = 0;
};
