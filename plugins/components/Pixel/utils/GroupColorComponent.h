#ifndef _GROUP_COLOR_COMPONENT_H_
#define _GROUP_COLOR_COMPONENT_H_

#include "../../component.h"
#include "./ToggleColor.h"

class GroupColorComponent : public Component {
protected:
    bool isActive = true;
    float inactiveColorRatio = 0.5f;

    std::vector<ToggleColor*> colors;

    void updateColors()
    {
        for (auto& color : colors) {
            color->toggle(isActive);
        }
    }

    void setInactiveColorRatio(float ratio)
    {
        inactiveColorRatio = ratio;
        for (auto& color : colors) {
            color->darkness(inactiveColorRatio);
        }
    }

public:
    GroupColorComponent(ComponentInterface::Props props, std::vector<ToggleColor*> colors)
        : Component(props)
        , colors(colors)
    {
    }

    void onGroupChanged(int8_t index) override
    {
        bool shouldActivate = false;
        if (group == index || group == -1) {
            shouldActivate = true;
        }
        if (shouldActivate != isActive) {
            isActive = shouldActivate;
            updateColors();
            renderNext();
        }
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "INACTIVE_COLOR_RATIO") == 0) {
            inactiveColorRatio = atof(value);
            setInactiveColorRatio(inactiveColorRatio);
            return true;
        }

        return false;
    }
};

#endif