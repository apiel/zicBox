#pragma once

#include "componentInterface.h"
#include "drawInterface.h"
#include "motionInterface.h"
#include "utils/VisibilityContext.h"
// #include "utils/VisibilityGroup.h" // TODO
#include "valueInterface.h"

#include <stdlib.h>

class Component : public ComponentInterface {
protected:
    VisibilityContext visibilityContext;
    // VisibilityGroup visibilityGroup; // TODO

public:
    Component(ComponentInterface::Props props)
        : ComponentInterface(props)
    {
    }

    ValueInterface* watch(ValueInterface* value)
    {
        if (value != NULL) {
            values.push_back(value);
        }
        return value;
    }

    virtual void clear() override
    {
        draw.filledRect(position, size, { draw.styles.colors.background });
    };

    virtual void render() override { }
    virtual void initView(uint16_t counter) override { }

    virtual void renderNext() override
    {
        if (isVisible()) {
            view->pushToRenderingQueue(this);
        }
    }

    virtual bool isVisible() override { return visibilityContext.visible; }

    virtual void onUpdate(ValueInterface* value) override
    {
        renderNext();
    }

    virtual void onMotion(MotionInterface& motion) override
    {
    }

    virtual void handleMotion(MotionInterface& motion) override
    {
        if (!motion.originIn({ position, size })) {
            return;
        }

        if (motion.in({ position, size })) {
            if (motion.isStarting() && group != -1) {
                view->setGroup(group);
            }
            onMotion(motion);
        }
    }

    virtual void onMotionRelease(MotionInterface& motion) override
    {
    }

    virtual void handleMotionRelease(MotionInterface& motion) override
    {
        onMotionRelease(motion);
    }

    virtual void onEncoder(int id, int8_t direction) override
    {
    }

    virtual void onKey(uint16_t id, int key, int8_t state, unsigned long now) override
    {
    }

    virtual bool config(char* key, char* value) override
    {
        return false;
    }
    virtual bool baseConfig(char* key, char* value) override
    {
        if (strcmp(key, "GROUP") == 0) {
            group = atoi(value);
            // By default set active group to 0
            onGroupChanged(0);
            return true;
        }

        if (strcmp(key, "TRACK") == 0) {
            track = atoi(value);
            return true;
        }

        if (strcmp(key, "ID") == 0) {
            id = value;
            return true;
        }

        if (visibilityContext.config(key, value)) {
            return true;
        }

        // TODO
        // if (visibilityGroup.config(key, value)) {
        //     return true;
        // }

        return config(key, value);
    }

    virtual void onGroupChanged(int8_t index) override
    {
    }

    virtual void onContext(uint8_t index, float value) override
    {
        if (visibilityContext.onContext(index, value)) {
            renderNext();
        }
    }
};
