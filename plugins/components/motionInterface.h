/** Description:
This technical file defines a fundamental "Motion Interface," which acts as a standardized blueprint for any component within the system designed to move or track movement.

**Core Purpose:**
The interface establishes a mandatory contract, ensuring that all physical or virtual objects capable of motion (like robots, motors, or graphical elements) share the exact same set of properties and commands. This allows the rest of the program to interact with various moving parts using a single, unified structure.

**Key Information Tracked (Properties):**
Every object based on this blueprint must maintain several pieces of data: a unique ID number, an ID for its hardware sensor (like an encoder), its current location (Position), its initial starting location (Origin), and a large internal count for tracking steps or cycles.

**Required Actions (Behaviors):**
The blueprint specifies several critical actions that must be implemented by any component using it:
1.  **Setup and Identification:** Setting the unique IDs and initializing the starting position.
2.  **Control:** Commanding the component to move to new coordinates.
3.  **Status Checks:** Determining if a process is just beginning, and checking if the current position or the original starting point falls inside a predefined rectangular boundary.

**How it Works:**
This file is purely a definition; it contains no actual code for movement. Instead, it serves as a strict template. Other specific modules (e.g., a "Car" module or a "RobotArm" module) will adopt this blueprint and write their specialized code to fulfill these required movement and reporting actions.

sha: f0f8410b139d93680f819175546b2ed08a849966599d7055f6416531f83d253a 
*/
#pragma once

#include "baseInterface.h"

class MotionInterface
{
public:
    int id = -1;
    int8_t encoderId = -1;
    Point position = {0, 0};
    Point origin = {0, 0};
    unsigned long counter = 0;

    virtual void setId(int id) = 0;
    virtual void setEncoderId(int8_t id) = 0;
    virtual void init(int id, int x, int y) = 0;
    virtual void move(int x, int y) = 0;
    virtual bool isStarting() = 0;
    virtual bool in(Rect rect) = 0;
    virtual bool originIn(Rect rect) = 0;
};
