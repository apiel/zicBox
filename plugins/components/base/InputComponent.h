#ifndef _UI_COMPONENT_INPUT_H_
#define _UI_COMPONENT_INPUT_H_

#include "../component.h"

class InputComponent : public Component {
protected:
    int fontSize = 12;
    Point textPosition;

    struct Colors {
        Color background;
        Color text;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ draw.darken(color, 0.6), color });
    }

    const int margin;

public:
    char value[256] = "";
    std::function<void()> onUpdate = []() {};

    InputComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.grey))
        , margin(styles.margin)
    {
    }

    void render()
    {
        draw.filledRect(
            { position.x + margin, position.y + margin },
            { size.w - 2 * margin, size.h - 2 * margin },
            colors.background);

        draw.text(textPosition, value, colors.text, fontSize);
    }

    void setFontSize(int _fontSize)
    {
        fontSize = _fontSize;
        textPosition = { position.x + 2, (int)(position.y + size.h * 0.5f - (fontSize * 0.5f)) };
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "COLOR") == 0) {
            colors = getColorsFromColor(draw.getColor(value));
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
