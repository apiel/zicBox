#ifndef _GROUP_SCROLL_CONTAINER_H_
#define _GROUP_SCROLL_CONTAINER_H_

#include "./ComponentContainer.h"

class ScrollGroupContainer : public ComponentContainer {
public:
    ScrollGroupContainer(ViewInterface* view, std::string name, Point position, Size size)
        : ComponentContainer(view, name, position, size)
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

    void onGroupChanged(int8_t index) override
    {
        printf("scroll group change: %d\n", index);
        // if (index != activeGroup) {
        //     activeGroup = index;
        //     if (index == 2) {
        //         ComponentContainer* container = getContainer("Scroll");
        //         if (container) {
        //             // container->position.y = -100;
        //         }
        //     }
        // }
    }

    bool config(char* key, char* params) override
    {
        printf("ScrollGroupContainer config: %s %s\n", key, params);
        return false;
    }
};

#endif