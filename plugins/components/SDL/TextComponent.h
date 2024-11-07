#ifndef _UI_COMPONENT_TEXT_H_
#define _UI_COMPONENT_TEXT_H_

#include "../component.h"
#include <string>

/*md
## TextComponent

Draw a text on the view.
*/
class TextComponent : public Component {
protected:
    std::string text;

    uint8_t size = 12;

    struct Colors {
        Color text;
    } colors;

    enum Align {
        LEFT,
        CENTER,
        RIGHT
    } align
        = CENTER;

    const char* fontPath;

public:
    TextComponent(ComponentInterface::Props props)
        : Component(props)
        , colors({ props.draw.styles.colors.white })
        , fontPath(props.draw.styles.font.regular)
    {
    }

    void render()
    {
        if (align == LEFT) {
            draw.text(position, text, size, { colors.text, fontPath });
        } else if (align == CENTER) {
            draw.textCentered(position, text, size, { colors.text, fontPath });
        } else {
            draw.textRight(position, text, size, { colors.text, fontPath });
        }
    }

    bool config(char* key, char* value)
    {
        /*md - `TEXT: Hello World` set the text. */
        if (strcmp(key, "TEXT") == 0) {
            text = value;
            return true;
        }

        /*md - `COLOR: #FFFFFF` set the text color. Default is white. */
        if (strcmp(key, "COLOR") == 0) {
            colors.text = draw.getColor(value);
            return true;
        }

        /*md - `FONT_SIZE: 10` set the font size. Default is 12. */
        if (strcmp(key, "FONT_SIZE") == 0) {
            size = atoi(value);
            return true;
        }

        /*md - `ALIGN: left` set the text align. Default is center. */
        if (strcmp(key, "ALIGN") == 0) {
            if (strcmp(value, "left") == 0) {
                align = LEFT;
            } else if (strcmp(value, "center") == 0) {
                align = CENTER;
            } else if (strcmp(value, "right") == 0) {
                align = RIGHT;
            }
            return true;
        }

        /*md - `BOLD: true` set bold text. */
        if (strcmp(key, "BOLD") == 0) {
            if (strcmp(value, "true") == 0) {
                fontPath = styles.font.bold;
            }
            return true;
        }

        return false;
    }
};

#endif
