#ifndef _VISIBILITY_CONTAINER_H_
#define _VISIBILITY_CONTAINER_H_

#include "./ComponentContainer.h"
#include "plugins/components/componentInterface.h"

/*md
## Visibility

Visibility is a container that show/hide the components for a given group index.
*/

class VisibilityContainer : public ComponentContainer {
protected:
    int group = -1;
    bool isGroupVisible = true;

    int16_t contextIndex = -1;
    float contextValue = 0;
    bool isContextVisible = true;

    enum Condition {
        SHOW_WHEN,
        SHOW_WHEN_NOT,
        SHOW_WHEN_OVER,
        SHOW_WHEN_UNDER
    } cond = SHOW_WHEN;

    Color bgColor;

public:
    VisibilityContainer(ViewInterface* view, std::string name, Point position, Size size)
        : ComponentContainer(view, name, position, size)
        , bgColor(view->draw.styles.colors.background)
    {
    }

    void renderBackground()
    {
        if (isGroupVisible && isContextVisible) {
            view->draw.filledRect(position, size, { bgColor });
        }
    }

    bool isVisible(Point initialPosition, Size componentSize) override
    {
        return isGroupVisible && isContextVisible;
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
        if (index == contextIndex) {
            if (cond == SHOW_WHEN_NOT) {
                isContextVisible = value != contextValue;
            } else if (cond == SHOW_WHEN_OVER) {
                isContextVisible = value > contextValue;
            } else if (cond == SHOW_WHEN_UNDER) {
                isContextVisible = value < contextValue;
            } else {
                isContextVisible = value == contextValue;
            }
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
        if (strcmp(key, "VISIBILITY_CONTEXT") == 0) {
            contextIndex = atoi(strtok(value, " "));
            std::string condStr = strtok(NULL, " ");
            contextValue = atof(strtok(NULL, " "));
            if (condStr == "SHOW_WHEN_NOT") {
                cond = SHOW_WHEN_NOT;
            } else if (condStr == "SHOW_WHEN_OVER") {
                cond = SHOW_WHEN_OVER;
            } else if (condStr == "SHOW_WHEN_UNDER") {
                cond = SHOW_WHEN_UNDER;
            } else {
                cond = SHOW_WHEN;
            }
            return true;
        }

        return false;
    }
};

#endif