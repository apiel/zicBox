#ifndef _UI_COMPONENT_RECT_H_
#define _UI_COMPONENT_RECT_H_

#include "../component.h"
#include <string>

/*md
## SDL RectComponent

Draw a rectangle on the view. Can be use to group elements together.
*/
class RectComponent : public Component {
protected:
    struct Colors {
        Color background;
        Color border;
    } colors;

    bool drawBorder = false;
    bool drawBackground = false;
    int radius = 0;

public:
    RectComponent(ComponentInterface::Props props)
        : Component(props)
    {
        colors.background = props.draw.styles.colors.background;
        colors.border = colors.background;
    }

    void render()
    {
        if (radius) {
            if (drawBackground) {
                draw.filledRect(position, size, radius, { colors.background });
            }
            if (drawBorder) {
                draw.rect(position, size, radius, { colors.border });
            }
        } else {
            if (drawBackground) {
                draw.filledRect(position, size, { colors.background });
            }
            if (drawBorder) {
                draw.rect(position, size, { colors.border });
            }
        }
    }

    bool config(char* key, char* value)
    {
        /*md - `BACKGROUND: #333333` set the background color and activate background drawing. */
        if (strcmp(key, "BACKGROUND") == 0) {
            colors.background = draw.getColor(value);
            drawBackground = true;
            return true;
        }

        /*md - `BORDER: #333333` set the border color and activate border drawing. */
        if (strcmp(key, "BORDER") == 0) {
            colors.border = draw.getColor(value);
            drawBorder = true;
            return true;
        }

        /*md - `RADIUS: 15` set the radius of the rounded corners. */
        if (strcmp(key, "RADIUS") == 0) {
            radius = atoi(value);
            return true;
        }

        return false;
    }
};

#endif
