#ifndef _CONTAINERS_H_
#define _CONTAINERS_H_

#include "./ScrollGroupContainer.h"
#include "./GroupVisibilityContainer.h"

#include <string>

ComponentContainer* newContainer(std::string type, ViewInterface* view, std::string name, Point position, Size size)
{
    if (type == "ScrollGroupContainer") {
        return new ScrollGroupContainer(view, name, position, size);
    }

    if (type == "GroupVisibilityContainer") {
        return new GroupVisibilityContainer(view, name, position, size);
    }

    return NULL;
}

#endif