#ifndef _UI_PIXEL_COMPONENT_SEP_H_
#define _UI_PIXEL_COMPONENT_SEP_H_

#include "../component.h"
#include "../utils/color.h"
#include "utils/GroupColorComponent.h"

/*md
## Separator

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/separator.png" />

Separator component is used to display a separator between components.
*/

class SeparatorComponent : public GroupColorComponent {
    Color bgColor;
    ToggleColor color;

    std::string title;

    bool drawLine = true;

public:
    SeparatorComponent(ComponentInterface::Props props)
        : GroupColorComponent(props, { { "COLOR", &color } })
        , bgColor(styles.colors.background)
        , color(darken(styles.colors.text, 0.5), inactiveColorRatio)
    {
        updateColors();
    }

    void render() override
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { bgColor });
            if (drawLine) {
                draw.line({ relativePosition.x, relativePosition.y + (int)(size.h * 0.5) },
                    { relativePosition.x + size.w, relativePosition.y + (int)(size.h * 0.5) },
                    { color.color });
            }
            if (!title.empty()) {
                Point textPos = { relativePosition.x + (int)(size.w * 0.5), relativePosition.y + (int)(size.h * 0.5) - 4 };
                draw.filledRect({ textPos.x - (int)(8 * title.length() * 0.5) - 4, relativePosition.y },
                    { (int)title.length() * 8 + 8, size.h },
                    { bgColor });
                draw.textCentered(textPos, title, 8, { color.color });
            }
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        /*md - `TITLE: title` is the title of the component. */
        if (strcmp(key, "TITLE") == 0) {
            title = value;
            return true;
        }

        /*md - `DRAW_LINE: true/false` is if the line should be drawn (default: true). */
        if (strcmp(key, "DRAW_LINE") == 0) {
            drawLine = strcmp(value, "true") == 0;
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(value);
            return true;
        }

        /*md - `COLOR: color` is the color of the component. */
        return GroupColorComponent::config(key, value);
    }
};

#endif
