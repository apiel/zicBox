/** Description:
This C++ header file defines a robust user interface element: the `ButtonComponent`. Think of it as a highly customizable push button on a digital screen, designed to trigger specific actions within an application, particularly those related to graphics and audio control.

The component manages its own visual appearance, displaying either a text label or an icon. It tracks its state, changing colors instantly to provide visual feedback when a user interacts with it (e.g., the background darkens when pressed).

Its core functionality revolves around user interaction. The button handles input from both touchscreen events (motion/release) and physical keypad presses. Crucially, it distinguishes between a simple, quick press and a "long press" (holding the button down for a set time).

This sophistication allows for assigning up to four distinct behaviors: actions that trigger immediately upon pressing, upon releasing (standard or after a long press), or during the continuous long press.

The button’s versatility comes from its powerful configuration system. Users can link these button events to complex system functions. For instance, pressing the button can be configured to switch the application’s current screen, control musical instruments by sending digital signals (like MIDI notes), or directly adjust settings within audio processing components.

In essence, the `ButtonComponent` acts as a central translator, turning simple user clicks or key taps into complex and configurable application commands.

sha: f182ae9ccf7d409e249622eba1500db886104f8aef1ab8fd1f026b07b509b028 
*/
#ifndef _UI_COMPONENT_BUTTON_H_
#define _UI_COMPONENT_BUTTON_H_

#include "../base/Icon.h"
#include "../base/KeypadLayout.h"
#include "../component.h"
#include "../utils/color.h"

#include <functional>
#include <string>

/*md
## SDL Button

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/Button.png" />

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
        return Colors({ darken(color, 0.6), darken(color, 0.3), color, color });
    }

    const int margin;

public:
    std::function<void()> onPress = []() {};
    std::function<void()> onLongPress = []() {};
    std::function<void()> onRelease = []() {};
    std::function<void()> onLongPressRelease = []() {};

    ButtonComponent(ComponentInterface::Props props)
        : Component(props)
        , icon(props.view->draw)
        , colors(getColorsFromColor({ 0x80, 0x80, 0x80, 255 }))
        , margin(styles.margin)
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            /*md **Keyboard actions**: */
            /*md - `trigger` is used to trigger the button. */
            if (action == "trigger") {
                func = [&](KeypadLayout::KeyMap& keymap) {
                    if (keymap.pressedTime != -1) {
                        handlePress();
                    } else {
                        handleRelease();
                    } };
            }
            return func;
        })
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
            { isPressed ? colors.pressedBackground : colors.background });

        renderLabel();
    }

    void setLabel(std::string _label)
    {
        label = _label;
        renderLabel = icon.get(label, labelPosition, fontSize, colors.icon, Icon::CENTER);
        if (!renderLabel) {
            renderLabel = [&]() {
                draw.textCentered(labelPosition, label.c_str(), fontSize, { colors.title });
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
            colors.pressedBackground = darken(colors.background, 0.3);
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
                view->setView(name);
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

        /*md - `ON_PRESS: &SHIFT true/false` set shift value. */
        // TODO support shift with different value, e.g.: &SHIFT:2 or &SHIFT:123 or even &SHIFT:2:1
        if (strcmp(action, "&SHIFT") == 0) {
            char* shiftValue = strtok(NULL, " ");
            if (strcmp(shiftValue, "true") == 0) {
                event = [this]() { setContext(0, 1); };
            } else if (strcmp(shiftValue, "false") == 0) {
                event = [this]() { setContext(0, 0); };
            }
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
                audioPluginHandler->sendEvent(id, -1);
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
    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        // printf("onKey %d %d %d\n", id, key, state);
        keypadLayout.onKey(id, key, state, now);
    }

    void initView(uint16_t counter)
    {
        keypadLayout.renderKeypadColor();
    }
};

#endif
