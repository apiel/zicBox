#ifndef _UI_COMPONENT_BUTTON_BASE_H_
#define _UI_COMPONENT_BUTTON_BASE_H_

#include "../component.h"
#include "Icon.h"
#include <string>

#include <functional>

class ButtonBaseComponent : public Component {
protected:
    Point labelPosition;
    int fontSize = 12;

    Icon icon;

    bool isPressed = false;

    std::string label = "";

    std::function<void()> renderLabel = []() {};

    void handlePress(std::function<void()>& event, bool pressed)
    {
        isPressed = pressed;
        renderNext();
        event();
    }

    struct Colors {
        Color background;
        Color pressedBackground;
        Color title;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ draw.darken(color, 0.6), draw.darken(color, 0.3), color });
    }

    const int margin;

public:
    std::function<void()> onPress = []() {};
    std::function<void()> onRelease = []() {};
    
    ButtonBaseComponent(ComponentInterface::Props props)
        : Component(props)
        , icon(props.draw)
        , colors(getColorsFromColor(styles.colors.blue))
        , margin(styles.margin)
    {
        setFontSize(fontSize);
    }

    void render()
    {
        draw.filledRect(
            { position.x + margin, position.y + margin },
            { size.w - 2 * margin, size.h - 2 * margin },
            isPressed ? colors.pressedBackground : colors.background);

        renderLabel();
    }

    void setLabel(std::string _label)
    {
        label = _label;
        renderLabel = icon.get(label, labelPosition, fontSize, colors.title, Icon::CENTER);
        if (!renderLabel) {
            renderLabel = [&]() {
                draw.textCentered(labelPosition, label.c_str(), colors.title, fontSize);
            };
        }
    }

    void setFontSize(int _fontSize)
    {
        fontSize = _fontSize;
        labelPosition = { (int)(position.x + size.w * 0.5f), (int)(position.y + size.h * 0.5f - (fontSize * 0.5f)) };
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "LABEL") == 0) {
            setLabel(value);
            return true;
        }

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

    void onMotion(MotionInterface& motion)
    {
        handlePress(onPress, true);
    }

    void onMotionRelease(MotionInterface& motion)
    {
        if (motion.originIn({ position, size })) {
            handlePress(onRelease, false);
        }
    }
};

#endif
