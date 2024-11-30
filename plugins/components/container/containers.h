#ifndef _CONTAINERS_H_
#define _CONTAINERS_H_

#include "./ScrollContainer.h"

#include <string>

ComponentContainer* newContainer(std::string type, std::string name, Point position, Size size)
{
    if (type == "ScrollContainer") {
        return new ScrollContainer(name, position, size);
    }

    return NULL;
}

#endif