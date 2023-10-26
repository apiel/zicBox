#ifndef _UI_COMPONENT_H_
#define _UI_COMPONENT_H_

#include "drawInterface.h"
#include "motionInterface.h"
#include "valueInterface.h"
#include "componentInterface.h"

#include <stdlib.h>

class Component : public ComponentInterface
{
protected:
    virtual void render() = 0;

    ValueInterface *val(ValueInterface *value)
    {
        values.push_back(value);
        return value;
    }

public:
    Component(ComponentInterface::Props &props)
        : ComponentInterface(props)
    {
    }

    virtual void triggerRenderer()
    {
        if (needRendering)
        {
            render();
            needRendering = false;
            draw.next();
        }
    }

    virtual void renderNext()
    {
        needRendering = true;
    }

    virtual void onUpdate(ValueInterface *value)
    {
        renderNext();
    }

    virtual void onMotion(MotionInterface &motion)
    {
    }

    virtual void handleMotion(MotionInterface &motion)
    {
        if (!motion.originIn({position, size}))
        {
            return;
        }

        if (motion.in({position, size}))
        {
            if (motion.isStarting() && group != -1)
            {
                setGroup(group);
            }
            onMotion(motion);
        }
    }

    virtual void onMotionRelease(MotionInterface &motion)
    {
    }

    virtual void handleMotionRelease(MotionInterface &motion)
    {
        onMotionRelease(motion);
    }

    virtual void onEncoder(int id, int8_t direction)
    {
    }

    virtual bool config(char *key, char *value)
    {
        return false;
    }
    virtual bool baseConfig(char *key, char *value)
    {
        if (strcmp(key, "GROUP") == 0)
        {
            group = atoi(value);
            return true;
        }
        return config(key, value);
    }

    virtual void onGroupChanged(int8_t index)
    {
    }
};

#endif
