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
