#ifndef _UI_COMPONENT_FILTER_H_
#define _UI_COMPONENT_FILTER_H_

#include "component.h"
#include <string>

class FilterComponent : public Component {
protected:
    ValueInterface* cutoff = NULL;
    ValueInterface* resonance = NULL;

    void setCutoff(const char* pluginName, const char* key)
    {
        cutoff = val(getPlugin(pluginName).getValue(key));
    }

    void setResonance(const char* pluginName, const char* key)
    {
        resonance = val(getPlugin(pluginName).getValue(key));
    }

    struct Colors {
        Color background;
        Color value;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ draw.darken(color, 0.75), color });
    }

public:
    FilterComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
    {
    }

    void render()
    {
        draw.filledRect(position, size, colors.background);

        if (cutoff != NULL && resonance != NULL) {
        }
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "CUTOFF") == 0) {
            char* pluginName = strtok(value, " ");
            char* keyValue = strtok(NULL, " ");
            setCutoff(pluginName, keyValue);
            return true;
        }

        if (strcmp(key, "RESONANCE") == 0) {
            char* pluginName = strtok(value, " ");
            char* keyValue = strtok(NULL, " ");
            setResonance(pluginName, keyValue);
            return true;
        }

        if (strcmp(key, "COLOR") == 0) {
            colors = getColorsFromColor(draw.getColor(value));
            return true;
        }

        return false;
    }
};

#endif
