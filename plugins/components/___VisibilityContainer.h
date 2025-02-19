#ifndef _VISIBILITY_CONTAINER_H_
#define _VISIBILITY_CONTAINER_H_

#include "./ComponentContainer.h"
#include "plugins/components/componentInterface.h"
#include "plugins/components/utils/VisibilityContext.h"

#include <vector>

class VisibilityContainer : public ComponentContainer {
protected:
    int group = -1;
    bool isGroupVisible = true;

    VisibilityContext visibility;

    Color bgColor;

public:
    VisibilityContainer(ViewInterface* view, std::string name, Point position, Size size)
        : ComponentContainer(view, name, position, size)
        , bgColor(view->draw.styles.colors.background)
    {
    }

    void renderBackground()
    {
        if (isGroupVisible && visibility.visible) {
            view->draw.filledRect(position, size, { bgColor });
        }
    }

    bool isVisible(Point initialPosition, Size componentSize) override
    {
        return isGroupVisible && visibility.visible;
    }

    bool updateCompontentPosition(Point initialPosition, Size componentSize, Point& relativePosition) override
    {
        if (isVisible(initialPosition, componentSize)) {
            return ComponentContainer::updateCompontentPosition(initialPosition, componentSize, relativePosition);
        }
        return false;
    }

    void onGroupChanged(int8_t index) override
    {
        bool shouldActivate = false;
        if (group == index || group == -1) {
            shouldActivate = true;
        }
        if (shouldActivate != isGroupVisible) {
            isGroupVisible = shouldActivate;
            renderBackground();
            view->renderAllNext(this);
        }
    }

    void onContext(uint8_t index, float value) override
    {
        if (visibility.onContext(index, value)) {
            renderBackground();
            view->renderAllNext(this);
        }
    }

    bool config(char* key, char* value) override
    {
        if (strcmp(key, "CONTAINER_BACKGROUND_COLOR") == 0) {
            bgColor = view->draw.getColor(value);
            return true;
        }

        if (strcmp(key, "VISIBILITY_GROUP") == 0) {
            group = atoi(value);
            return true;
        }

        return false;
    }
};

#endif