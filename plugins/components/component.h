#ifndef _UI_COMPONENT_H_
#define _UI_COMPONENT_H_

#include "componentInterface.h"
#include "drawInterface.h"
#include "motionInterface.h"
#include "valueInterface.h"

#include <stdlib.h>

class Component : public ComponentInterface {
public:
    Component(ComponentInterface::Props props)
        : ComponentInterface(props)
    {
    }

    ValueInterface* watch(ValueInterface* value)
    {
        if (value) {
            values.push_back(value);
        }
        return value;
    }

    virtual void clear(){
        draw.filledRect(position, size, { draw.styles.colors.background });
    };

    virtual void render() { }
    virtual void initView(uint16_t counter) { }

    virtual void renderNext()
    {
        if (active && pushToRenderingQueue) {
            pushToRenderingQueue(this);
        }
    }

    virtual void onUpdate(ValueInterface* value)
    {
        renderNext();
    }

    virtual void onMotion(MotionInterface& motion)
    {
    }

    virtual void handleMotion(MotionInterface& motion)
    {
        if (!motion.originIn({ position, size })) {
            return;
        }

        if (motion.in({ position, size })) {
            if (motion.isStarting() && group != -1) {
                setGroup(group);
            }
            onMotion(motion);
        }
    }

    virtual void onMotionRelease(MotionInterface& motion)
    {
    }

    virtual void handleMotionRelease(MotionInterface& motion)
    {
        onMotionRelease(motion);
    }

    virtual void onEncoder(int id, int8_t direction)
    {
    }

    virtual void onKey(uint16_t id, int key, int8_t state)
    {
    }

    virtual bool config(char* key, char* value)
    {
        return false;
    }
    virtual bool baseConfig(char* key, char* value)
    {
        if (strcmp(key, "GROUP") == 0) {
            group = atoi(value);
            // By default set active group to 0
            onGroupChanged(0);
            return true;
        }

        if (strcmp(key, "VISIBILITY") == 0) {
            visibility = atoi(value);
            if (visibility != 0) { // Maybe not the best...
                active = false;
            }
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

        return config(key, value);
    }

    virtual void onGroupChanged(int8_t index)
    {
    }

    virtual void onVisibilityChanged(int8_t index)
    {
        if (visibility == index || visibility == -1) {
            active = true;
            renderNext();
        }
    }
};

#endif
