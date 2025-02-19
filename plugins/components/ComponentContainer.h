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
};

#endif