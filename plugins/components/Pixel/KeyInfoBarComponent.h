#ifndef _UI_PIXEL_COMPONENT_KEY_INFO_BAR_H_
#define _UI_PIXEL_COMPONENT_KEY_INFO_BAR_H_

#include "../base/Icon.h"
#include "../base/KeypadLayout.h"
#include "../component.h"

#include <cmath>
#include <string>
#include <vector>

/*md
## KeyInfoBar

KeyInfoBar components provide some information about the current kaypad layout.
*/
class KeyInfoBarComponent : public Component {
protected:
    Icon icon;
    KeypadLayout keypadLayout;
    int8_t activeGroup = 0;

    Color textColor;

    void handleButton(int8_t id, int8_t state)
    {
        printf("handleButton: %d %d\n", id, state);

        if (state == 1) {
            if (id == 0) {
                setGroup(activeGroup - 1);
            } else if (id == 5) {
                setGroup(activeGroup + 1);
            } else {
                AudioPlugin* plugin = &getPlugin("SynthDrum23", 1);
                plugin->noteOn(60, 1.0f);
            }
        }
    }

public:
    void addKeyMap(KeypadInterface* controller, uint16_t controllerId, uint8_t key, int param, std::string action, uint8_t color)
    {
        if (action == "item") {
            keypadLayout.mapping.push_back(
                { controller, controllerId, key, param,
                    [&](int8_t state, KeypadLayout::KeyMap& keymap) { handleButton(keymap.param, state); },
                    color, [&](KeypadLayout::KeyMap& keymap) { return keymap.color == 255 ? 10 : keymap.color; } });
        }
    }

    KeyInfoBarComponent(ComponentInterface::Props props)
        : Component(props)
        , icon(props.draw)
        , keypadLayout(getController, [&](KeypadInterface* controller, uint16_t controllerId, int8_t key, int param, std::string action, uint8_t color) { addKeyMap(controller, controllerId, key, param, action, color); })
        , textColor(styles.colors.text)
    {
        buttonWidth = size.w / 5.0f;
        buttonStartX = buttonWidth * 0.5f;
    }

protected:
    int buttonWidth = 40;
    int buttonStartX = 14;

    string text[10] = {
        "&icon::arrowUp", "FX", "Synth", "Live", "Track",
        "&icon::arrowDown", "mod", "...", "&icon::pause", "&icon::play"
    };
    void renderRow(int y, int startIndex)
    {
        for (int i = 0; i < 5; i++) {
            Point textPos = { relativePosition.x + buttonStartX + i * buttonWidth, y };
            if (!icon.render(text[i + startIndex], textPos, 8, {}, Icon::CENTER)) {
                draw.textCentered(textPos, text[i + startIndex], 8);
            }
        }
    }

public:
    void render()
    {
        if (updatePosition()) {
            renderRow(relativePosition.y, 0);
            renderRow(relativePosition.y + 12, 5);
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        if (keypadLayout.config(key, value)) {
            return true;
        }

        /*md - `TEXT_COLOR: color` is the color of the text. */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            textColor = draw.getColor(value);
            return true;
        }

        return false;
    }

    void onKey(uint16_t id, int key, int8_t state)
    {
        keypadLayout.onKey(id, key, state);
    }

    // void onGroupChanged(int8_t index) override
    // {
    //     activeGroup = index;
    // }

    void onGroupChanged(int8_t index) override
    {
        if (index != activeGroup) {
            activeGroup = index;
            if (index == 2) {
                
            }
        }
    }

    void initView(uint16_t counter)
    {
        keypadLayout.renderKeypad();
    }
};

#endif
