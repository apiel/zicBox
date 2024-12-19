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
    bool isContextVisible = true;
    int8_t visibleWhen = -1;
    int8_t hideWhen = -1;

public:
    VisibilityContainer(ViewInterface* view, std::string name, Point position, Size size)
        : ComponentContainer(view, name, position, size)
    {
    }

    bool updateCompontentPosition(Point initialPosition, Size componentSize, Point& relativePosition) override
    {
        if (isGroupVisible && isContextVisible) {
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
            view->renderAllNext(this);
        }
    }

    void onContext(uint8_t index, float value) override
    {
        if (index == contextIndex) {
            if (hideWhen != -1) {
                isContextVisible = value != hideWhen;
            }
            if (visibleWhen != -1) {
                isContextVisible = value == visibleWhen;
            }
            view->renderAllNext(this);
        }
    }

    bool config(char* key, char* value) override
    {
        /*md - `VISIBILITY_GROUP: 0` the group index to show/hide the components. */
        if (strcmp(key, "VISIBILITY_GROUP") == 0) {
            group = atoi(value);
            return true;
        }

        /*md - `VISIBILITY_CONTEXT: index SHOW/HIDE value` the context index to show/hide the components for a given value. */
        if (strcmp(key, "VISIBILITY_CONTEXT") == 0) {
            contextIndex = atoi(strtok(value, " "));
            if (strcmp(strtok(NULL, " "), "HIDE") == 0) {
                hideWhen = atoi(strtok(NULL, " "));
            } else {
                visibleWhen = atoi(strtok(NULL, " "));
            }
            return true;
        }

        return false;
    }
};

#endif