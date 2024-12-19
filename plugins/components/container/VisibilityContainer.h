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
    bool isVisible = true;

public:
    VisibilityContainer(ViewInterface* view, std::string name, Point position, Size size)
        : ComponentContainer(view, name, position, size)
    {
    }

    bool updateCompontentPosition(Point initialPosition, Size componentSize, Point& relativePosition) override
    {
        if (isVisible) {
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
        if (shouldActivate != isVisible) {
            isVisible = shouldActivate;
            // printf("[%s] current group: %d inccoming group: %d visible: %s\n", name.c_str(), group, index, isVisible ? "true" : "false");
            std::vector<void*>* components = view->getComponents();
            for (void* c : *components) {
                ComponentInterface* component = (ComponentInterface*)c;
                if (component->container == this) {
                    component->renderNext();
                }
            }
        }
    }

    bool config(char* key, char* value) override
    {
        /*md - `VISIBILITY_GROUP: 0` the group index to show/hide the components. */
        if (strcmp(key, "VISIBILITY_GROUP") == 0) {
            group = atoi(value);
            return true;
        }

        return false;
    }
};

#endif