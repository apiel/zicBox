/** Description:
## Keypad Controller Blueprint Analysis

**Purpose and Role**

This file serves as a standardized blueprint, or "interface," for managing any electronic device that operates as an array of buttons, such as a music launchpad, specialty controller, or specialized keyboard. It establishes the mandatory rules and features required for any specific hardware driver to correctly integrate into a larger system.

**Core Mechanism**

The blueprint inherits fundamental management functions from a more general "Controller Interface." It then adds characteristics specific to keypads, specializing the generic control rules for button-based devices.

**Key Components and Features:**

1.  **Mandatory Capabilities:** The interface includes a crucial rule defining the required functionality. This rule, known as `setButton`, dictates that any concrete keypad driver built upon this blueprint *must* provide a method to change the color of a specific button, identified by its unique number (ID). The blueprint itself doesn't contain the programming logic for *how* the color signal is sent to the physical hardware; it only guarantees the ability exists.
2.  **Color Support:** It explicitly declares a property named `hasColor` and sets it to true. This signals to the overall system that this type of device supports visual feedback, typically through illuminated or colored buttons (LEDs).
3.  **Initialization:** The code ensures that when a new keypad object is created, it is properly configured using its unique identification number and general system properties inherited from the base controller interface.

**How the Code Works (Basic Idea):**

Instead of defining a complete, working keypad, this code defines the contract. It sets the baseline expectations and required functions (like the ability to control button colors) that *every* future driver for a physical keypad must adhere to. This ensures that different hardware devices can be swapped out easily while the rest of the application remains consistent.

sha: 756b920d0908041ad3d8d2bed03a9a2ffa78489ed894ceb9a11af73cd0764cc0 
*/
#ifndef _KEYPAD_INTERFACE_H_
#define _KEYPAD_INTERFACE_H_

#include "controllerInterface.h"

class KeypadInterface : public ControllerInterface {
public:
    bool hasColor = true;

    KeypadInterface(ControllerInterface::Props& props, uint16_t id)
        : ControllerInterface(props, id)
        // , activeLayout(layouts[0])
    {
    }

    virtual void setButton(int id, uint8_t color) = 0;
};

#endif