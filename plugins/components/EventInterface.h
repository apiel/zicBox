/** Description:
This header file defines the `EventInterface`, which serves as a critical blueprint or contract for managing all forms of user interaction within the software system.

**Purpose of the Interface**
This is a pure interface, meaning it doesn't contain the actual code logic, but rather specifies a set of functions that any component responsible for handling inputs (like a plug-in or a display driver) must implement. It dictates *what* events the system recognizes, ensuring standardized processing across the entire application.

**Key Event Management Functions**

The interface is structured to handle various types of inputs:

1.  **Movement and Gesture Tracking:** Functions are defined to manage continuous actions, such as touch screen input or motion tracking, including the start of movement (`onMotion`) and the end of the action (`onMotionRelease`).
2.  **Dedicated Hardware Inputs:** Specific functions manage physical controls:
    *   `onEncoder`: Handles rotary dials or input wheels, tracking which dial was turned, the direction of movement, and the precise timing of the event.
    *   `onKey`: Manages button presses or keyboard events, identifying the specific key and its current state (pressed or released).
3.  **Application State:** A `resize` function ensures the system can correctly recalculate and scale its user interface elements whenever the layout or screen resolution changes.

**Component Description**

The interface also includes a nested structure (`EncoderPosition`) used to describe the physical attributes of specific rotary controls, detailing their identifier, size, position, and behavior within the application layout.

In summary, this blueprint guarantees that all components interacting with the user follow the same rules for receiving and processing events, from physical buttons and dials to dynamic screen movements.

sha: 085a3acfd766477f353676d0bc2052ae11d5350bd442d96dc5b1cdc5eef90dd4 
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
    virtual void resize(float xFactor, float yFactor) = 0;

    struct EncoderPosition {
        int8_t id;
        Size size;
        Point pos;
        bool allowMotionScroll = true;
    };
    virtual int8_t getEncoderId(int32_t x, int32_t y, bool isMotion = false) = 0;
};
