#ifndef _GROUP_COLOR_COMPONENT_H_
#define _GROUP_COLOR_COMPONENT_H_

#include "plugins/components/component.h"
#include "./ToggleColor.h"


// Give name to colors, so group color can handle config...

class GroupColorComponent : public Component {
protected:
    bool isActive = true;
    float inactiveColorRatio = 0.5f;



    void updateColors()
    {
        for (auto& color : colors) {
            color.color->toggle(isActive);
        }
    }

    void setInactiveColorRatio(float ratio)
    {
        inactiveColorRatio = ratio;
        for (auto& color : colors) {
            color.color->darkness(inactiveColorRatio);
        }
    }

public:
    struct NamedColor {
        std::string name;
        ToggleColor* color;
    };

    std::vector<NamedColor> colors;
    
    GroupColorComponent(ComponentInterface::Props props, std::vector<NamedColor> colors)
        : Component(props)
        , colors(colors)
    {
    }

    GroupColorComponent(ComponentInterface::Props props, std::vector<NamedColor> colors, float inactiveColorRatio)
        : Component(props)
        , colors(colors)
        , inactiveColorRatio(inactiveColorRatio)
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
        for (auto& color : colors) {
            if (strcmp(key, color.name.c_str()) == 0) {
                color.color->setColor(draw.getColor(value), inactiveColorRatio);
                return true;
            }
        }

        if (strcmp(key, "INACTIVE_COLOR_RATIO") == 0) {
            inactiveColorRatio = atof(value);
            setInactiveColorRatio(inactiveColorRatio);
            return true;
        }

        return false;
    }
};

#endif