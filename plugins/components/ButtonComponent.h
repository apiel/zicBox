#ifndef _UI_COMPONENT_BUTTON_H_
#define _UI_COMPONENT_BUTTON_H_

#include "component.h"
#include <string>

#include <functional>

class ButtonComponent : public Component {
protected:
    char* label = NULL;
    int fontSize = 12;
    Point labelPosition;

    bool isPressed = false;

    std::function<void()> onPress = []() {};
    std::function<void()> onRelease = []() {};

    void render()
    {
        draw.filledRect(
            { position.x + margin, position.y + margin },
            { size.w - 2 * margin, size.h - 2 * margin },
            isPressed ? colors.pressedBackground : colors.background);

        draw.textCentered(labelPosition, label, colors.title, fontSize);
    }

    void set(std::function<void()>& event, char* config)
    {
        char* pluginName = strtok(config, " ");

        if (strcmp(pluginName, "$SET_VIEW") == 0) {
            char* name = strtok(NULL, " ");
            char* nameCopy = new char[strlen(name) + 1];
            strcpy(nameCopy, name);
            event = [nameCopy, this]() {
                this->setView(nameCopy);
            };
            return;
        }

        char* key = strtok(NULL, " ");
        char* targetValue = strtok(NULL, " ");

        ValueInterface* value = val(getPlugin(pluginName).getValue(key));
        if (value != NULL && label == NULL) {
            label = (char*)value->label();
        }

        float target = atof(targetValue);
        event = [value, target]() {
            value->set(target);
        };
    }

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
    ButtonComponent(ComponentInterface::Props& props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
        , margin(styles.margin)
        , labelPosition({ (int)(position.x + size.w * 0.5f), (int)(position.y + size.h * 0.5f - (fontSize * 0.5f)) })
    {
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "ON_PRESS") == 0) {
            printf("value: %s\n", value);
            set(onPress, value);
            return true;
        }

        if (strcmp(key, "ON_RELEASE") == 0) {
            printf("value: %s\n", value);
            set(onRelease, value);
            return true;
        }

        if (strcmp(key, "LABEL") == 0) {
            // maybe we check that the current point is not pointing to value->label()
            // and free the label, but why should we assign multiple time a label...
            label = new char[strlen(value) + 1];
            strcpy(label, value);
            return true;
        }

        if (strcmp(key, "COLOR") == 0) {
            colors = getColorsFromColor(draw.getColor(value));
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
