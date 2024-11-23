#ifndef _UI_PIXEL_COMPONENT_KEY_INFO_BAR_H_
#define _UI_PIXEL_COMPONENT_KEY_INFO_BAR_H_

#include "../base/Icon.h"
#include "../base/KeypadLayout.h"
#include "../component.h"

#include <cmath>
#include <string>
#include <vector>

class KeyInfoBarComponent : public Component {
protected:
    Icon icon;
    KeypadLayout keypadLayout;

    void handleButton(int8_t id, int8_t state)
    {
        printf("handleButton: %d %d\n", id, state);

        if (state == 1) {
            AudioPlugin* plugin = &getPlugin("SynthDrum23", 1);
            plugin->noteOn(60, 1.0f);
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
    {
        buttonWidth = size.w / 5.0f;
        buttonStartX = buttonWidth * 0.5f;
    }

protected:
    int buttonWidth = 40;
    int buttonStartX = 14;

    string text[10] = {
        "FX", "mod", "src", "Perf", "Tck",
        "&icon::play", "&icon::arrowUp", "&icon::arrowDown", "...", "&icon::pause"
    };
    void renderRow(int y, int startIndex)
    {
        for (int i = 0; i < 5; i++) {
            Point textPos = { position.x + buttonStartX + i * buttonWidth, y };
            if (!icon.render(text[i + startIndex], textPos, 8, {}, Icon::CENTER)) {
                draw.textCentered(textPos, text[i + startIndex], 8);
            }
        }
    }

public:
    void render()
    {
        renderRow(position.y, 0);
        renderRow(position.y + 10, 5);
    }

    bool config(char* key, char* value)
    {
        if (keypadLayout.config(key, value)) {
            return true;
        }

        return false;
    }

    void onKey(uint16_t id, int key, int8_t state)
    {
        keypadLayout.onKey(id, key, state);
    }

    void initView(uint16_t counter)
    {
        keypadLayout.renderKeypad();
    }
};

#endif
