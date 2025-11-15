#pragma once

#include "./draw.h"
#include "motion.h"
#include "plugins/components/EventInterface.h"

#ifndef MAX_SCREEN_MOTION
#define MAX_SCREEN_MOTION 5
#endif

class DrawDesktop : public Draw {
protected:
    int windowX = 0;
    int windowY = 0;

    Motion motions[MAX_SCREEN_MOTION];

#if SFML_VERSION_MAJOR == 2
    uint8_t emulateEncoderId = 0;
#endif

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
            motion->move(x, y);
            view->onMotion(*motion);
        }
    }

    void handleMotionDown(EventInterface* view, int x, int y, int id)
    {
        if (id < 0) return;

        MotionInterface* motion = getOldestMotion();
        motion->init(id, x, y);
        view->onMotion(*motion);
    }

    void handleMotionUp(EventInterface* view, int x, int y, int id)
    {
        if (id < 0) return;

        MotionInterface* motion = getMotion(id);
        if (motion) {
            motion->move(x, y);
            view->onMotionRelease(*motion);
            motion->setId(-1);
        }
    }

    std::vector<Rect> zoneEncoders = {};
    uint8_t getEmulatedEncoderId(int32_t x, int32_t y)
    {
        for (uint8_t i = 0; i < zoneEncoders.size(); i++) {
            if (inRect(zoneEncoders[i], { x, y })) {
                return i;
            }
        }
        return 255;
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

        if (config.contains("zonesEncoders")) {
            for (auto zoneEncoder : config["zonesEncoders"]) {
                int x = zoneEncoder[0];
                int y = zoneEncoder[1];
                int w = zoneEncoder[2];
                int h = zoneEncoder[3];
                zoneEncoders.push_back({ { x, y }, { w, h } });
            }
        }

        Draw::config(config);
    }
};
