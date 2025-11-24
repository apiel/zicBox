/** Description:
This C++ header defines a foundational blueprint, or contract, named `EventInterface`. Its primary purpose is to standardize how different parts of an application receive and respond to user inputs and environmental changes.

This interface is entirely "abstract," meaning it does not contain any functional code itself. Instead, it mandates a specific set of functions that any component intending to handle inputs (like a display driver or a UI element) must implement.

### Core Responsibilities of the Interface:

The `EventInterface` is designed to handle four main categories of interaction events:

1.  **Motion Input:** Functions like `onMotion` and `onMotionRelease` are required to track continuous movements, such as screen swipes or mouse drags, including when the user stops the action.
2.  **Rotary Dials (Encoders):** The `onEncoder` function handles inputs from digital dials or scroll wheels. It tracks which dial was moved, the direction of rotation (forward or backward), and the exact time or count when the movement occurred.
3.  **Key Input:** The `onKey` function is mandatory for receiving standard button or keyboard presses, noting the button identifier and the state (pressed or released).
4.  **Display Scaling:** The `resize` function ensures the component can properly adjust its internal coordinates when the overall window or display area changes size.

### Managing Virtual Dials:

The interface also defines a specialized data structure (`EncoderPosition`) which helps track the location and size of rotary dials displayed on the screen. Functions are included to allow the system to query the physical location of these dials and determine which specific dial is being interacted with based on screen coordinates.

sha: 37147053c51f7164b41891ba21b1e29197ac0e3a1ed1f688c8c01e20ba5c6102 
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
    virtual const std::vector<EventInterface::EncoderPosition> getEncoderPositions() = 0; // might not even expose this in event interface.
    virtual int8_t getEncoderId(int32_t x, int32_t y, bool isMotion = false) = 0;
};
