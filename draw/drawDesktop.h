/** Description:
This file defines the `DrawDesktop` component, which is a specialized part of the application responsible for managing how graphics are drawn and, crucially, how user interactions are processed in a desktop environment.

It acts as a sophisticated translator between basic system input (like mouse clicks and cursor movement) and the application's functional events.

**Core Functionality:**

The component is designed to track multiple, simultaneous inputs (analogous to multi-touch on a screen) using a dedicated tracking system. When a user interacts (e.g., clicks and drags), the system immediately records the starting point and assigns a unique identifier to that interaction.

**Handling User Motion:**

1.  **Press (Down):** When an interaction begins, the system identifies if the touch/click landed on a standard movable area or on a dedicated control element, such as a virtual scrolling knob (an "encoder").
2.  **Move (Drag):** As the input moves, the system decides the appropriate action:
    *   If it’s a standard drag, it simply updates the view to reflect the movement.
    *   If it’s interacting with a virtual knob, it measures the distance moved horizontally and converts that distance into discrete scrolling steps or "ticks." This allows a continuous mouse movement to translate into specific rotational inputs for UI elements.
3.  **Release (Up):** When the interaction ends, the system sends a final release notification and clears the tracking data for that specific input.

Additionally, the component manages initial application settings, such as reading configuration data to determine the precise starting position of the application window on the screen.

sha: eb81b4017c8a0f433798f9008076fbc75cc96a16fb37d0c61c7ee29f875739d5
*/
#pragma once

#include <unordered_map>

#include "draw/draw.h"
#include "draw/drawRenderer.h"
#include "motion.h"
#include "plugins/components/EventInterface.h"

class DrawDesktop : public DrawRenderer {
protected:
    Draw& draw;

    int windowX = 0;
    int windowY = 0;

    std::unordered_map<int, Motion> motions;

    MotionInterface* getMotion(int id)
    {
        auto it = motions.find(id);
        return (it != motions.end()) ? &it->second : nullptr;
    }

    void handleMotion(EventInterface* view, int x, int y, int id)
    {
        if (id < 0) return;

        MotionInterface* motion = getMotion(id);
        if (motion) {
            if (motion->encoderId < 0) {
                motion->move(x, y);
                view->onMotion(*motion);
                // logDebug("motion %d: %d %d", id, motion->position.x, motion->position.y);
            } else {
                int dx = x - motion->position.x;
                // int dy = y - motion->position.y;

                int delta = dx / 10; // sensitivity 10px per unit
                // int delta = dx / 5; // sensitivity 5px per unit

                if (delta != 0) {
                    view->onEncoder(motion->encoderId, delta, 0); // set tick to 0 to encGetScaledDirection is skipped
                }
            }
        }
    }

    void handleMotionDown(EventInterface* view, int x, int y, int id)
    {
        if (id < 0) return;

        MotionInterface* motion = &motions[id];
        motion->init(id, x, y);

        motion->setEncoderId(view->getEncoderId(x, y, true));
        if (motion->encoderId < 0) {
            view->onMotion(*motion);
        }
    }

    void handleMotionUp(EventInterface* view, int x, int y, int id)
    {
        if (id < 0) return;

        MotionInterface* motion = getMotion(id);
        if (motion) {
            if (motion->encoderId < 0) {
                motion->move(x, y);
                view->onMotionRelease(*motion);
            }
            motions.erase(id);
        }
    }

public:
    DrawDesktop(Draw& draw)
        : draw(draw)
    {
    }

    void config(nlohmann::json& config) override
    {
        if (config.contains("windowPosition")) {
            auto wp = config["windowPosition"];
            if (wp.contains("x") && wp.contains("y")) {
                windowX = wp["x"];
                windowY = wp["y"];
                // logDebug("windowPosition: %d %d", windowX, windowY);
            }
        }
    }
};
