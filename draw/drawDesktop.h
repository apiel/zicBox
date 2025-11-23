#pragma once

#include <unordered_map>

#include "draw/draw.h"
#include "motion.h"
#include "plugins/components/EventInterface.h"

class DrawDesktop : public Draw {
protected:
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
    DrawDesktop(Styles& styles)
        : Draw(styles)
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

        Draw::config(config);
    }
};
