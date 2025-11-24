/** Description:
This code defines a specialized tracking system named `Motion`, designed to handle and record user inputs like mouse movement or touch gestures. It serves as the core logic for managing active input events within a software application.

### How the Motion Tracker Works

1.  **Identification and Setup:** The tracker is assigned unique identifiers (`id`) to distinguish it from other simultaneous inputs (e.g., multiple fingers on a screen). The `init` function records the exact coordinates where the touch or cursor movement began (the "origin") and resets the internal step counter.
2.  **Tracking Movement:** The `move` function continuously updates the current position coordinates as the input changes. Simultaneously, it increments a counter, logging how many steps or updates have occurred since the action started.
3.  **Boundary Checks:** A primary feature is the ability to determine if a point is within a specific boundary. The tracker can instantly verify if the *current* position or the *starting* (origin) position falls inside a defined rectangular area. This is essential for checking if a user clicked or touched a specific interface element or button.
4.  **Status Reporting:** It can report whether the movement has just begun (`isStarting`), meaning the counter is still zero, or if the input has been actively moving.

In essence, this code provides a robust framework for accurately capturing, measuring, and validating active user interaction coordinates.

sha: 74d9dd0c1e1b4bad31eb3b80f96d0275e1a1e20af0dea1eeb1d7f6d8121c469a 
*/
#pragma once

#include "plugins/components/utils/inRect.h"
#include "plugins/components/motionInterface.h"

// Touch and mouse motion
class Motion: public MotionInterface
{
public:
    void setId(int id) override
    {
        this->id = id;
    }

    void setEncoderId(int8_t id) override
    {
        encoderId = id;
    }

    void init(int id, int x, int y) override
    {
        this->id = id;
        position.x = x;
        position.y = y;
        origin.x = x;
        origin.y = y;
        counter = 0;
    }

    void move(int x, int y) override
    {
        position.x = x;
        position.y = y;
        counter++;
    }

    bool isStarting() override
    {
        return counter == 0;
    }

    bool in(Rect rect) override
    {
        return inRect(rect, position);
    }

    bool originIn(Rect rect) override
    {
        return inRect(rect, origin);
    }
};
