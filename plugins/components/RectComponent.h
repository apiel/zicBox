#ifndef _UI_COMPONENT_RECT_H_
#define _UI_COMPONENT_RECT_H_

#include "component.h"
#include <string>

class RectComponent : public Component {
protected:
    struct Colors {
        Color background;
        Color border;
    } colors;

    bool drawBorder = false;

public:
    RectComponent(ComponentInterface::Props props)
        : Component(props)
    {
        colors.background = props.draw.styles.colors.background;
        colors.border = colors.background;
    }

    void render()
    {
        draw.filledRect(position, size, colors.background);
        if (drawBorder) {
            draw.rect(position, size, colors.border);
        }
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "COLOR") == 0) {
            colors.background = draw.getColor(value);
            return true;
        }

        if (strcmp(key, "BORDER") == 0) {
            colors.border = draw.getColor(value);
            drawBorder = true;
            return true;
        }

        return false;
    }
};

#endif
