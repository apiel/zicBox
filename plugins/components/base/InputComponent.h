#ifndef _UI_COMPONENT_INPUT_H_
#define _UI_COMPONENT_INPUT_H_

#include "../component.h"

class InputComponent : public Component {
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

    Colors getColorsFromColor(Color color)
    {
        return Colors({ draw.darken(color, 0.6), draw.darken(color, 0.3), color });
    }

    const int margin;

public:
    char value[256] = "";

    InputComponent(ComponentInterface::Props props)
        : Component(props)
        , textPosition({ position.x + 2, (int)(position.y + size.h * 0.5f - (fontSize * 0.5f)) })
        , colors(getColorsFromColor(styles.colors.grey))
        , margin(styles.margin)
    {
        setFontSize(fontSize);

        // jobRendering = [this](unsigned long now) {
        //     if (now - lastRendering > 1000) {
        //         printf("InputComponent::jobRendering\n");
        //         lastRendering = now;
        //         renderNext();
        //     }
        // };
    }

    void render()
    {
        draw.filledRect(
            { position.x + margin, position.y + margin },
            { size.w - 2 * margin, size.h - 2 * margin },
            colors.background);

        draw.line(
            { position.x + margin, position.y + size.h - margin },
            { position.x + size.w - margin, position.y + size.h - margin },
            colors.line);

        if (strlen(value) > 0) {
            draw.text(textPosition, value, colors.text, fontSize);
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
