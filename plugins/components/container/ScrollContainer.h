#ifndef _SCROLL_CONTAINER_H_
#define _SCROLL_CONTAINER_H_

#include "./ComponentContainer.h"

class ScrollContainer : public ComponentContainer {
public:
    ScrollContainer(std::string name, Point position, Size size)
        : ComponentContainer(name, position, size)
    {
    }
};

#endif