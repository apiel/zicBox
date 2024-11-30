#ifndef _COMPONENT_CONTAINER_H_
#define _COMPONENT_CONTAINER_H_

#include "plugins/components/drawInterface.h"

class ComponentContainer {
public:
    std::string name;
    Point position = { 0, 0 };
    Size size = { 0, 0 };

    ComponentContainer() { }

    ComponentContainer(std::string name, Point position, Size size)
        : name(name)
        , position(position)
        , size(size)
    {
    }

    virtual bool updateCompontentPosition(Point initialPosition, Size size, Point& relativePosition)
    {
        int x = initialPosition.x + position.x;
        int y = initialPosition.y + position.y;
        if (x >= 0 || y >= 0) {
            relativePosition = { x, y };
            return true;
        }
        return false;
    }

    virtual void onGroupChanged(int8_t index)
    {
    }
};

#endif