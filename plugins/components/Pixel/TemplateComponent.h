#ifndef _UI_PIXEL_COMPONENT_RECT_H_
#define _UI_PIXEL_COMPONENT_RECT_H_

#include "plugins/components/component.h"

#include <string>

/*md
## Rect

Rect components to draw a rectangle.
*/

class RectComponent : public Component {
protected:
    Color background;

public:
    RectComponent(ComponentInterface::Props props)
        : Component(props)
        , background(styles.colors.background)
    {
    }
    void render()
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { background });
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        /*md - `BACKGROUND_COLOR: color` is the color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            background = draw.getColor(value);
            return true;
        }

        return false;
    }
};

#endif
