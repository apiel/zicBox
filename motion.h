#ifndef _UI_MOTION_H_
#define _UI_MOTION_H_

#include "helpers/inRect.h"
#include "plugins/components/motionInterface.h"

// Touch and mouse motion
class Motion: public MotionInterface
{
public:
    void setId(int id)
    {
        this->id = id;
    }

    void init(int id, int x, int y)
    {
        this->id = id;
        position.x = x;
        position.y = y;
        origin.x = x;
        origin.y = y;
        counter = 0;
    }

    void move(int x, int y)
    {
        position.x = x;
        position.y = y;
        counter++;
    }

    bool isStarting()
    {
        return counter == 0;
    }

    bool in(Rect rect)
    {
        return inRect(rect, position);
    }

    bool originIn(Rect rect)
    {
        return inRect(rect, origin);
    }
};

#endif
