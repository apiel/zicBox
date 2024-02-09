#ifndef _UI_COMPONENT_RECT_H_
#define _UI_COMPONENT_RECT_H_

#include "component.h"
#include <string>

/*md
## RectComponent

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
                draw.filledRect({ position.x + radius, position.y }, { size.w - 2 * radius, size.h }, colors.background);
                draw.filledRect({ position.x, position.y + radius }, { size.w, size.h - 2 * radius }, colors.background);
                draw.filledPie({ position.x + radius, position.y + radius }, radius, 180, 270, colors.background);
                draw.filledPie({ position.x + size.w - radius, position.y + radius }, radius, 270, 360, colors.background);
                draw.filledPie({ position.x + radius, position.y + size.h - radius }, radius, 90, 180, colors.background);
                draw.filledPie({ position.x + size.w - radius, position.y + size.h - radius }, radius, 0, 90, colors.background);
            }
            if (drawBorder) {
                draw.arc({ position.x + radius, position.y + radius }, radius, 180, 270, colors.border);
                draw.line({ position.x, position.y + radius }, { position.x, position.y + size.h - radius }, colors.border);
                draw.arc({ position.x + size.w - radius, position.y + radius }, radius, 270, 360, colors.border);
                draw.line({ position.x + radius, position.y }, { position.x + size.w - radius, position.y }, colors.border);
                draw.arc({ position.x + radius, position.y + size.h - radius }, radius, 90, 180, colors.border);
                draw.line({ position.x + radius, position.y + size.h }, { position.x + size.w - radius, position.y + size.h }, colors.border);
                draw.arc({ position.x + size.w - radius, position.y + size.h - radius }, radius, 0, 90, colors.border);
                draw.line({ position.x + size.w, position.y + size.h - radius }, { position.x + size.w, position.y + radius }, colors.border);
            }
        } else {
            if (drawBackground) {
                draw.filledRect(position, size, colors.background);
            }
            if (drawBorder) {
                draw.rect(position, size, colors.border);
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
