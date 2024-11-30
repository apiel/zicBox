#ifndef _GROUP_SCROLL_CONTAINER_H_
#define _GROUP_SCROLL_CONTAINER_H_

#include "./ComponentContainer.h"
#include "plugins/components/componentInterface.h"

class ScrollGroupContainer : public ComponentContainer {
protected:
    std::vector<int> yPositionPerGroup;

    Color bgColor;

    Point originPosition;

public:
    ScrollGroupContainer(ViewInterface* view, std::string name, Point position, Size size)
        : ComponentContainer(view, name, position, size)
        , bgColor(view->draw.styles.colors.background)
        , originPosition(position)
    {
    }

    void initContainer() override
    {
        int16_t lastGroupId = -1;
        std::vector<void*>* components = view->getComponents();
        for (void* c : *components) {
            ComponentInterface* component = (ComponentInterface*)c;
            if (component->group > lastGroupId) {
                lastGroupId = component->group;
            }
        }
        int heightSum = 0;
        for (int i = 0; i <= lastGroupId; i++) {
            int minY = 99999999;
            int maxY = 0;
            for (void* c : *components) {
                ComponentInterface* component = (ComponentInterface*)c;
                if (component->group == i) {
                    if (component->position.y + component->size.h > maxY) {
                        maxY = component->position.y + component->size.h;
                    }
                    if (component->position.y < minY) {
                        minY = component->position.y;
                    }
                }
            }
            printf("[%d] minY: %d maxY: %d size.h: %d (%d) pos.y: %d\n", i, minY, maxY, size.h, (int)(size.h * 0.5), originPosition.y);
            if (maxY > size.h * 0.6) {
                yPositionPerGroup.push_back(-(minY - size.h * 0.6));
                printf("push: %d\n", yPositionPerGroup.back());
            } else {
                yPositionPerGroup.push_back(0);
            }
        }

        // yPositionPerGroup[2] = -60;
        // yPositionPerGroup[3] = -60;
    }

    bool updateCompontentPosition(Point initialPosition, Size componentSize, Point& relativePosition) override
    {
        int x = initialPosition.x + position.x;
        int y = initialPosition.y + position.y;
        if (x >= 0 && y >= 0 && y + componentSize.h < originPosition.y + size.h) {
            relativePosition = { x, y };
            return true;
        }

        return false;
    }

    int lastY = 0;
    void onGroupChanged(int8_t index) override
    {
        // printf("scroll group change: %d\n", index);
        if (index >= 0 && index < yPositionPerGroup.size()) {
            position.y = yPositionPerGroup[index];
            if (position.y != lastY) {
                lastY = position.y;
                printf("set scroll container position.y: %d\n", position.y);

                view->draw.filledRect(originPosition, size, { bgColor });
                // view->draw.filledRect(originPosition, size, { 0xFF, 0xFF, 0xFF });

                // maybe need to make something in view for this?
                std::vector<void*>* components = view->getComponents();
                for (void* c : *components) {
                    ComponentInterface* component = (ComponentInterface*)c;
                    component->renderNext();
                }
            }
        }
    }

    bool config(char* key, char* value) override
    {
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = view->draw.getColor(value);
            return true;
        }

        return false;
    }
};

#endif