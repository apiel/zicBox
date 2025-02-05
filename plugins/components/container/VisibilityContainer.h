#ifndef _VISIBILITY_CONTAINER_H_
#define _VISIBILITY_CONTAINER_H_

#include "./ComponentContainer.h"
#include "plugins/components/componentInterface.h"
#include "plugins/components/utils/VisibilityContext.h"

#include <vector>

/*md
## Visibility

Visibility is a container that show/hide the components for a given group index.
*/

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
        /*md - `CONTAINER_BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "CONTAINER_BACKGROUND_COLOR") == 0) {
            bgColor = view->draw.getColor(value);
            return true;
        }

        /*md - `VISIBILITY_GROUP: 0` the group index to show/hide the components. */
        if (strcmp(key, "VISIBILITY_GROUP") == 0) {
            group = atoi(value);
            return true;
        }

        /*md - `VISIBILITY_CONTEXT: index SHOW_WHEN/SHOW_WHEN_NOT/SHOW_WHEN_OVER/SHOW_WHEN_UNDER value` the context index to show/hide the components for a given value. */
        if (visibility.config(key, value)) {
            return true;
        }

        return false;
    }
};

#endif