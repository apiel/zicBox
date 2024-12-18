#ifndef _GROUP_VISIBILITY_CONTAINER_H_
#define _GROUP_VISIBILITY_CONTAINER_H_

#include "./ComponentContainer.h"
#include "plugins/components/componentInterface.h"

/*md
## GroupVisibility

GroupVisibility is a container that show/hide the components for a given group index.
*/

class GroupVisibilityContainer : public ComponentContainer {
protected:
    int group = -1;
    bool isVisible = true;

public:
    GroupVisibilityContainer(ViewInterface* view, std::string name, Point position, Size size)
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
            printf("[%s] current group: %d inccoming group: %d\n", name.c_str(), group, index);
            isVisible = shouldActivate;
            std::vector<void*>* components = view->getComponents();
            for (void* c : *components) {
                ComponentInterface* component = (ComponentInterface*)c;
                component->renderNext();
            }
        }
    }

    bool config(char* key, char* value) override
    {
        /*md - `GROUP: 0` the group index to show/hide the components. */
        if (strcmp(key, "GROUP") == 0) {
            group = atoi(value);
            return true;
        }

        return false;
    }
};

#endif