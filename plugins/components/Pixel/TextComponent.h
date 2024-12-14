#ifndef _UI_PIXEL_COMPONENT_TEXT_H_
#define _UI_PIXEL_COMPONENT_TEXT_H_

#include "../component.h"
#include "../utils/color.h"
#include "utils/GroupColorComponent.h"

/*md
## Text

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/text.png" />

Text component is used to display text.
*/

class TextComponent : public GroupColorComponent {
    Color bgColor;
    ToggleColor color;

    std::string text;

    bool centered = false;
    int fontSize = 8;
    void *font = NULL;

public:
    TextComponent(ComponentInterface::Props props)
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
            if (!text.empty()) {
                if (centered) {
                    Point textPos = { relativePosition.x + (int)(size.w * 0.5), relativePosition.y + (int)(size.h * 0.5) - 4 };
                    draw.textCentered(textPos, text, fontSize, { color.color, .font = font });
                } else {
                    draw.text({ relativePosition.x, relativePosition.y }, text, fontSize, { color.color, .font = font });
                }
            }
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        /*md - `TEXT: text` is the text of the component. */
        if (strcmp(key, "TEXT") == 0) {
            text = value;
            return true;
        }

        /*md - `CENTERED: true/false` is if the text should be centered (default: false). */
        if (strcmp(key, "CENTERED") == 0) {
            centered = strcmp(value, "true") == 0;
            return true;
        }

        /*md - `FONT_SIZE: size` is the font size of the component. */
        if (strcmp(key, "FONT_SIZE") == 0) {
            fontSize = atoi(value);
            return true;
        }

        /*md - `FONT: font` is the font of the component. */
        if (strcmp(key, "FONT") == 0) {
            font = draw.getFont(value);
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
