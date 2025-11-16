#pragma once

#include "motion.h"
#include "draw/draw.h"
#include "plugins/components/EventInterface.h"

#ifndef MAX_SCREEN_MOTION
#define MAX_SCREEN_MOTION 5
#endif

class DrawDesktop : public Draw {
protected:
    int windowX = 0;
    int windowY = 0;

    Motion motions[MAX_SCREEN_MOTION];

    MotionInterface* getMotion(int id)
    {
        for (int i = 0; i < MAX_SCREEN_MOTION; ++i) {
            if (motions[i].id == id) {
                return &motions[i];
            }
        }
        return nullptr;
    }

    MotionInterface* getOldestMotion()
    {
        MotionInterface* oldest = &motions[0];
        for (int i = 1; i < MAX_SCREEN_MOTION; ++i) {
            if (motions[i].id < oldest->id) {
                oldest = &motions[i];
            }
        }
        return oldest;
    }

    void handleMotion(EventInterface* view, int x, int y, int id)
    {
        if (id < 0) return;

        MotionInterface* motion = getMotion(id);
        if (motion) {
            if (motion->encoderId < 0) {
                motion->move(x, y);
                view->onMotion(*motion);
            } else {
                int dx = x - motion->position.x;
                // int dy = y - motion->position.y;

                int delta = dx / 10; // sensitivity 20px per unit

                if (delta != 0) {
                    view->onEncoder(motion->encoderId, delta, 0); // set tick to 0 to encGetScaledDirection is skipped
                }
            }
        }
    }

    void handleMotionDown(EventInterface* view, int x, int y, int id)
    {
        if (id < 0) return;

        MotionInterface* motion = getOldestMotion();
        motion->init(id, x, y);

        motion->setEncoderId(view->getEncoderId(x, y));
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
            motion->setId(-1);
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
