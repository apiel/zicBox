#ifndef _CONTAINERS_H_
#define _CONTAINERS_H_

#include "./ScrollGroupContainer.h"

#include <string>

ComponentContainer* newContainer(std::string type, std::string name, Point position, Size size)
{
    if (type == "ScrollGroupContainer") {
        return new ScrollGroupContainer(name, position, size);
    }

    return NULL;
}

#endif