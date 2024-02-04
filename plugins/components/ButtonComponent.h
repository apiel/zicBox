#ifndef _UI_COMPONENT_BUTTON_H_
#define _UI_COMPONENT_BUTTON_H_

#include "./base/KeypadLayout.h"
#include "base/Icon.h"
#include "component.h"

#include <functional>
#include <string>

/*md
## Button

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Button.png" />

Push button component that trigger an action when pressed and released.
*/
class ButtonComponent : public Component {
protected:
    Point labelPosition;
    int fontSize = 12;

    Icon icon;

    bool isPressed = false;
    bool isLongPress = false;
    unsigned long pressedTime = -1;

    std::string label = "";

    std::function<void()> renderLabel = []() {};

    KeypadLayout keypadLayout;

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

    ButtonComponent(ComponentInterface::Props props)
        : Component(props)
        , icon(props.draw)
        , colors(getColorsFromColor(styles.colors.grey))
        , margin(styles.margin)
        , keypadLayout(getController, [&](KeypadInterface* controller, uint16_t controllerId, int8_t key, int param, std::string action, uint8_t color) { addKeyMap(controller, controllerId, key, param, action, color); })
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
        renderLabel = icon.get(label, labelPosition, fontSize, colors.icon, Icon::CENTER);
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

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        /*md - `ON_PRESS: action` is used to set an action when the button is pressed, e.g: `ON_PRESS: &SET_VIEW name`.*/
        if (strcmp(key, "ON_PRESS") == 0) {
            // printf("value: %s\n", value);
            set(onPress, value);
            return true;
        }

        /*md - `ON_RELEASE: action` is used to set an action when the button is released, e.g: `ON_RELEASE: &SET_VIEW name`.*/
        if (strcmp(key, "ON_RELEASE") == 0) {
            // printf("value: %s\n", value);
            set(onRelease, value);
            return true;
        }

        /*md - `ON_LONG_PRESS: action` is used to set an action when the button is long pressed.*/
        if (strcmp(key, "ON_LONG_PRESS") == 0) {
            // printf("value: %s\n", value);
            set(onLongPress, value);
            return true;
        }

        /*md - `ON_LONG_PRESS_RELEASE: action` is used to set an action when the button is released when long pressed.*/
        if (strcmp(key, "ON_LONG_PRESS_RELEASE") == 0) {
            // printf("value: %s\n", value);
            set(onLongPressRelease, value);
            return true;
        }

        /*md - `LABEL: label` is used to set the text of the button. [Icons](https://github.com/apiel/zicBox/wiki/04-UI-Component-plugins#icons) can also be used, e.g.: `LABEL: &icon::play::filled`.*/
        if (strcmp(key, "LABEL") == 0) {
            setLabel(value);
            return true;
        }

        /*md - `COLOR: color` is used to set the color of the button.*/
        if (strcmp(key, "COLOR") == 0) {
            colors = getColorsFromColor(draw.getColor(value));
            return true;
        }

        /*md - `ICON_COLOR: color` is used to set the color of the button icon.*/
        if (strcmp(key, "ICON_COLOR") == 0) {
            colors.icon = draw.getColor(value);
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is used to set the background color of the button.*/
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            colors.background = draw.getColor(value);
            colors.pressedBackground = draw.darken(colors.background, 0.3);
            return true;
        }

        /*md - `PRESSED_BACKGROUND_COLOR: color` is used to set the background color of the button when pressed.*/
        if (strcmp(key, "PRESSED_BACKGROUND_COLOR") == 0) {
            colors.pressedBackground = draw.getColor(value);
            return true;
        }

        /*md - `FONT_SIZE: size` is used to set the font size of the button. Font size is also used to defined the height of the icon.*/
        if (strcmp(key, "FONT_SIZE") == 0) {
            setFontSize(atoi(value));
            return true;
        }

        if (keypadLayout.config(key, value)) {
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

protected:
    // Keep after for documentation order
    /*md **Actions**: */
    void set(std::function<void()>& event, char* config)
    {
        char* action = strtok(config, " ");

        /*md - `ON_PRESS: &SET_VIEW name` sets the specified view when the button is pressed. */
        if (strcmp(action, "&SET_VIEW") == 0) {
            std::string name = strtok(NULL, " ");
            event = [this, name]() {
                this->setView(name);
            };
            return;
        }

        /*md - `ON_PRESS: &NOTE_ON pluginName note velocity` send the note on the specified plugin when the button is pressed. */
        if (strcmp(action, "&NOTE_ON") == 0) {
            char* pluginName = strtok(NULL, " ");
            char* noteStr = strtok(NULL, " ");
            uint8_t note = atoi(noteStr);
            char* velocityStr = strtok(NULL, " ");
            float velocity = atof(velocityStr);
            AudioPlugin* plugin = &getPlugin(pluginName, track);
            event = [plugin, note, velocity]() {
                plugin->noteOn(note, velocity);
            };
            return;
        }

        /*md - `ON_PRESS: &NOTE_OFF pluginName note` send the note off the specified plugin when the button is pressed. */
        if (strcmp(action, "&NOTE_OFF") == 0) {
            char* pluginName = strtok(NULL, " ");
            char* noteStr = strtok(NULL, " ");
            uint8_t note = atoi(noteStr);
            AudioPlugin* plugin = &getPlugin(pluginName, track);
            event = [plugin, note]() {
                plugin->noteOff(note, 0);
            };
            return;
        }

        /*md - `ON_PRESS: &DATA pluginName dataId` send the data to the specified plugin when the button is pressed. */
        if (strcmp(action, "&DATA") == 0) {
            char* pluginName = strtok(NULL, " ");
            int dataId = atoi(strtok(NULL, " "));
            AudioPlugin* plugin = &getPlugin(pluginName, track);
            event = [plugin, dataId]() {
                plugin->data(dataId);
            };
            return;
        }

        /*md - `ON_PRESS: &AUDIO_EVENT id` send audio event when the button is pressed. */
        /*md    - `STOP` to send stop event to all tracks */
        /*md    - `START` to send start event to all tracks */
        /*md    - `PAUSE` to send pause event to all tracks */
        /*md    - `TOGGLE_PLAY_PAUSE` to send toggle play pause event to all tracks */
        if (strcmp(action, "&AUDIO_EVENT") == 0) {
            AudioEventType id = getEventTypeFromName(strtok(NULL, " "));
            event = [&, id]() {
                sendAudioEvent(id);
            };
            return;
        }

        /*md - `ON_PRESS: pluginName key value` sets the value of the specified plugin key when the button is pressed. */
        char* key = strtok(NULL, " ");
        char* targetValue = strtok(NULL, " ");

        ValueInterface* value = watch(getPlugin(action, track).getValue(key));
        if (value != NULL && label == "") {
            label = value->label();
        } else {
            // printf("No value %s %s found for button on track %d\n", action, key, track);
        }

        float target = atof(targetValue);
        event = [value, target]() {
            value->set(target);
        };
    }

public:
    /*md **Keyboard actions**: */
    void addKeyMap(KeypadInterface* controller, uint16_t controllerId, uint8_t key, int param, std::string action, uint8_t color)
    {
        /*md - `trigger` is used to trigger the button. */
        if (action == "trigger") {
            keypadLayout.mapping.push_back({ controller, controllerId, key, param, [&](int8_t state, KeypadLayout::KeyMap& keymap) {
                    if (state) {
                        handlePress();
                    } else {
                        handleRelease();
                    } }, color, [&](KeypadLayout::KeyMap& keymap) { return keymap.color || 20; } });
        }
    }

    void onKey(uint16_t id, int key, int8_t state)
    {
        keypadLayout.onKey(id, key, state);
    }
};

#endif
