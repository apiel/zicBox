#ifndef _GROUP_SCROLL_CONTAINER_H_
#define _GROUP_SCROLL_CONTAINER_H_

#include "./ComponentContainer.h"
#include "plugins/components/componentInterface.h"

/*md
## ScrollGroup

ScrollGroup is a container that scroll the grouped component together.
*/

class ScrollGroupContainer : public ComponentContainer {
protected:
    std::vector<int> yPositionPerGroup;

    Color bgColor;

    Point originPosition;

    bool scrollToCenter = false;

    int scrollOffset;

public:
    ScrollGroupContainer(ViewInterface* view, std::string name, Point position, Size size)
        : ComponentContainer(view, name, position, size)
        , bgColor(view->draw.styles.colors.background)
        , originPosition(position)
    {
        scrollOffset = size.h * 0.7;
    }

    void initContainer() override
    {
        int8_t lastGroupId = -1;
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
            // printf("[%d] minY: %d maxY: %d size.h: %d (%d) pos.y: %d\n", i, minY, maxY, size.h, (int)(size.h * 0.5), originPosition.y);
            if (maxY > scrollOffset) {
                int h = maxY - minY;
                if (scrollToCenter) {
                    int y = -(minY - (size.h - h) * 0.5);
                    yPositionPerGroup.push_back(y > 0 ? 0 : y);
                } else {
                    int y = -(minY - size.h * 0.6);
                    yPositionPerGroup.push_back(y > 0 ? 0 : y);
                }
            } else {
                yPositionPerGroup.push_back(0);
            }
        }
    }

    bool isVisible(Point initialPosition, Size componentSize) override
    {
        int x = initialPosition.x + position.x;
        int y = initialPosition.y + position.y;
        return x >= 0 && y >= 0 && y + componentSize.h < originPosition.y + size.h;
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
                // printf("set scroll container position.y: %d\n", position.y);

                view->draw.filledRect(originPosition, size, { bgColor });
                view->renderAllNext(this);
            }
        }
    }

    bool config(char* key, char* value) override
    {
        /*md - `CONTAINER_BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "CONTAINER_BACKGROUND_COLOR") == 0) {
            bgColor = view->draw.getColor(value);
            return true;
        }

        /*md - `SCROLL_TO_CENTER: true/false` scroll to center (default: false). */
        if (strcmp(key, "SCROLL_TO_CENTER") == 0) {
            scrollToCenter = strcmp(value, "true") == 0;
            return true;
        }

        /*md - `SCROLL_OFFSET: 100` set the where group component start to scroll. By default 70% from container height. */
        if (strcmp(key, "SCROLL_OFFSET") == 0) {
            scrollOffset = atoi(value);
            return true;
        }

        return false;
    }
};

#endif