#ifndef _VISIBILITY_CONTAINER_H_
#define _VISIBILITY_CONTAINER_H_

#include "./ComponentContainer.h"
#include "plugins/components/componentInterface.h"

#include <vector>

/*md
## Visibility

Visibility is a container that show/hide the components for a given group index.
*/

class VisibilityContainer : public ComponentContainer {
protected:
    int group = -1;
    bool isGroupVisible = true;

    // int16_t contextIndex = -1;
    // float contextValue = 0;
    // bool isContextVisible = true;

    enum Condition {
        SHOW_WHEN,
        SHOW_WHEN_NOT,
        SHOW_WHEN_OVER,
        SHOW_WHEN_UNDER
    } cond
        = SHOW_WHEN;

    struct ContextCondition {
        int16_t index;
        float value;
        Condition cond;
        bool isVisible = true;
    };

    std::vector<ContextCondition> contextConditions;
    bool isContextVisible = true;

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
        bool update = false;
        isContextVisible = true;
        for (auto& cond : contextConditions) {
            if (index == cond.index) {
                if (cond.cond == SHOW_WHEN_NOT) {
                    cond.isVisible = value != cond.value;
                } else if (cond.cond == SHOW_WHEN_OVER) {
                    cond.isVisible = value > cond.value;
                } else if (cond.cond == SHOW_WHEN_UNDER) {
                    cond.isVisible = value < cond.value;
                } else {
                    cond.isVisible = value == cond.value;
                }
                update = true;
            }
            isContextVisible = isContextVisible && cond.isVisible;
        }

        if (update) {
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
            ContextCondition context;
            context.index = atoi(strtok(value, " "));
            std::string condStr = strtok(NULL, " ");
            context.value = atof(strtok(NULL, " "));
            if (condStr == "SHOW_WHEN_NOT") {
                context.cond = SHOW_WHEN_NOT;
            } else if (condStr == "SHOW_WHEN_OVER") {
                context.cond = SHOW_WHEN_OVER;
            } else if (condStr == "SHOW_WHEN_UNDER") {
                context.cond = SHOW_WHEN_UNDER;
            } else {
                context.cond = SHOW_WHEN;
            }
            contextConditions.push_back(context);
            return true;
        }

        return false;
    }
};

#endif