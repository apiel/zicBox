#ifndef _SCROLL_CONTAINER_H_
#define _SCROLL_CONTAINER_H_

#include "./ComponentContainer.h"

class ScrollContainer : public ComponentContainer {
public:
    ScrollContainer(std::string name, Point position, Size size)
        : ComponentContainer(name, position, size)
    {
    }

    bool updateCompontentPosition(Point initialPosition, Size size, Point& relativePosition) override
    {
        int x = initialPosition.x + position.x;
        int y = initialPosition.y + position.y;
        if (x >= 0 || y >= 0) {
            relativePosition = { x, y };
            return true;
        }
        return false;
    }
};

#endif