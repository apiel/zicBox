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
    Color color;

    bool filled = true;

public:
    RectComponent(ComponentInterface::Props props)
        : Component(props)
        , color(styles.colors.background)
    {
    }
    void render()
    {
        if (updatePosition()) {
            if (filled) {
                draw.filledRect(relativePosition, size, { color });
            } else {
                draw.rect(relativePosition, size, { color });
            }
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        /*md - `COLOR: color` is the color of the component. */
        if (strcmp(key, "COLOR") == 0) {
            color = draw.getColor(value);
            return true;
        }

        /*md - `FILLED: true/false` is if the rectangle should be filled (default: true). */
        if (strcmp(key, "FILLED") == 0) {
            filled = strcmp(value, "true") == 0;
            return true;
        }

        return false;
    }
};

#endif
