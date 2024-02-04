#ifndef _UI_BASE_COMPONENT_BUTTON_H_
#define _UI_BASE_COMPONENT_BUTTON_H_

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
    bool isLongPress = false;
    unsigned long pressedTime = -1;

    std::string label = "";

    std::function<void()> renderLabel = []() {};

    void handlePress()
    {
        pressedTime = -1;
        isLongPress = false;
        isPressed = true;
        renderNext();
        onPress();
    }

    void handleRelease()
    {
        isPressed = false;
        renderNext();
        if (isLongPress) {
            onLongPressRelease();
        } else {
            onRelease();
        }
    }

    struct Colors {
        Color background;
        Color pressedBackground;
        Color title;
        Color icon;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ draw.darken(color, 0.6), draw.darken(color, 0.3), color, color });
    }

    const int margin;

public:
    std::function<void()> onPress = []() {};
    std::function<void()> onLongPress = []() {};
    std::function<void()> onRelease = []() {};
    std::function<void()> onLongPressRelease = []() {};

    ButtonBaseComponent(ComponentInterface::Props props)
        : Component(props)
        , icon(props.draw)
        , colors(getColorsFromColor(styles.colors.blue))
        , margin(styles.margin)
    {
        setFontSize(fontSize);

        jobRendering = [this](unsigned long now) {
            if (!isLongPress && isPressed) {
                if (pressedTime == -1) {
                    pressedTime = now;
                } else if (now - pressedTime > 500) {
                    isLongPress = true;
                    onLongPress();
                    renderNext();
                }
            }
        };
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

        if (strcmp(key, "ICON_COLOR") == 0) {
            colors.icon = draw.getColor(value);
            return true;
        }

        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            colors.background = draw.getColor(value);
            colors.pressedBackground = draw.darken(colors.background, 0.3);
            return true;
        }

        if (strcmp(key, "PRESSED_BACKGROUND_COLOR") == 0) {
            colors.pressedBackground = draw.getColor(value);
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
        if (!isPressed) {
            handlePress();
        }
    }

    void onMotionRelease(MotionInterface& motion)
    {
        if (motion.originIn({ position, size })) {
            handleRelease();
        }
    }
};

#endif
