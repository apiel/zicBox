/** Description:
This code provides the fundamental blueprint for creating a simple text input box within a user interface.

It acts as a container, managing all the properties necessary for an input field to display correctly. Crucially, it stores the actual text typed by the user, the size of the font, and defines three specific colors: one for the background of the box, one for the distinguishing line drawn beneath the text, and one for the text itself.

The component handles its own drawing process. One main function is dedicated to sketching the box and the underline, and then displaying the stored text value. A separate, specialized mechanism is used exclusively to control the text insertion cursor, making it blink reliably every half-second.

The design is flexible, allowing external controls to easily customize the input box's appearance by adjusting both the color scheme and the size of the font displayed inside the field.

sha: 8294ae86af8088ead97dc02cdb27343d2084643d773b52dbf3eb3eb7229b6484 
*/
#ifndef _UI_BASE_COMPONENT_INPUT_H_
#define _UI_BASE_COMPONENT_INPUT_H_

#include "../../component.h"
#include "../../utils/color.h"

#include <string>

class InputBaseComponent : public Component {
public:
    struct Colors {
        Color background;
        Color line;
        Color text;
    } colors;

protected:
    unsigned long lastRendering = 0;

    int fontSize = 12;
    Point textPosition;
    int cursorX = 0;

    bool cursorStateShown = false;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ darken(color, 0.6), darken(color, 0.3), color });
    }

    const int margin;

public:
    std::string value = "";

    InputBaseComponent(ComponentInterface::Props props)
        : Component(props)
        , textPosition({ position.x + 2, (int)(position.y + size.h * 0.5f - (fontSize * 0.5f)) })
        , colors(getColorsFromColor({ 0x80, 0x80, 0x80, 255 }))
        , margin(styles.margin)
    {
        setFontSize(fontSize);
        cursorX = textPosition.x;
    }

    void render()
    {
        draw.filledRect(
            { position.x + margin, position.y + margin },
            { size.w - 2 * margin, size.h - 2 * margin },
            { colors.background });

        draw.line(
            { position.x + margin, position.y + size.h - margin },
            { position.x + size.w - margin, position.y + size.h - margin },
            { colors.line });

        if (value.size() > 0) {
            cursorX = draw.text(textPosition, value.c_str(), fontSize, { colors.text });
        } else {
            cursorX = textPosition.x;
        }
    }

    void renderCursor(unsigned long now)
    {
        if (now - lastRendering > 500) {
            lastRendering = now;
            draw.line(
                { cursorX + 1, textPosition.y },
                { cursorX + 1, textPosition.y + fontSize },
                { cursorStateShown ? colors.background : colors.text });
            cursorStateShown = !cursorStateShown;
            draw.renderNext(); // call draw.renderNext instead renderNext, so the whole component is not rendered
        }
    }

    void setFontSize(int _fontSize)
    {
        fontSize = _fontSize;
        textPosition = { position.x + 10, (int)(position.y + size.h * 0.5f - (fontSize * 0.5f)) };
    }

    void setColors(Color color)
    {
        colors = getColorsFromColor(color);
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "COLOR") == 0) {
            setColors(draw.getColor(value));
            return true;
        }

        if (strcmp(key, "FONT_SIZE") == 0) {
            setFontSize(atoi(value));
            return true;
        }

        return false;
    }
};

#endif
