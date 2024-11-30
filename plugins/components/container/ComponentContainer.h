#ifndef _COMPONENT_CONTAINER_H_
#define _COMPONENT_CONTAINER_H_

#include "plugins/components/ViewInterface.h"
#include "plugins/components/drawInterface.h"

class ComponentContainer {
public:
    std::string name;
    Point position = { 0, 0 };
    Size size = { 0, 0 };
    ViewInterface* view = NULL;

    ComponentContainer() { }

    ComponentContainer(ViewInterface* view, std::string name, Point position, Size size)
        : view(view)
        , name(name)
        , position(position)
        , size(size)
    {
    }

    // lets not call it init() to not conflict with View::init()
    virtual void initContainer() { }

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

    virtual bool config(char* key, char* params)
    {
        return false;
    }
};

#endif