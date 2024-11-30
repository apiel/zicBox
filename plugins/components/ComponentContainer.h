#ifndef _COMPONENT_CONTAINER_H_
#define _COMPONENT_CONTAINER_H_

#include "drawInterface.h"

class ComponentContainer {
public:
    Point position = { 0, 0 };

    virtual bool updateCompontentPosition(Point initialPosition, Size size, Point& relativePosition) {
        int x = initialPosition.x + position.x;
        int y = initialPosition.y + position.y;
        if (x != relativePosition.x || y != relativePosition.y) {
            relativePosition = { x, y };
            return true;
        }
        return false;
    }
};

#endif