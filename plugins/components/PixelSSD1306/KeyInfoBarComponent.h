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
    void addKeyMap(KeypadInterface* controller, uint16_t controllerId, uint8_t key, std::string action, char* param, std::string actionLongPress, char* paramLongPress)
    {
        if (action == "item") {
            int* id = new int(atoi(param));
            keypadLayout.mapping.push_back(
                {
                    controller,
                    controllerId,
                    key,
                    [&](int8_t state, KeypadLayout::KeyMap& keymap) { handleButton(*(int*)keymap.param, state); },
                    id,
                });
        }
    }

    KeyInfoBarComponent(ComponentInterface::Props props)
        : Component(props)
        , icon(props.view->draw)
        , keypadLayout(getController, [&](KeypadInterface* controller, uint16_t controllerId, int8_t key, std::string action, char* param, std::string actionLongPress, char* paramLongPress) { addKeyMap(controller, controllerId, key, action, param, actionLongPress, paramLongPress); })
    {
    }

protected:
    string text[10] = {
        "FX", "mod", "src", "Perf", "Tck",
        "&icon::play", "&icon::arrowUp", "&icon::arrowDown", "...", "&icon::pause"
    };
    void renderRow(int y, int startIndex)
    {
        for (int i = 0; i < 5; i++) {
            Point textPos = { position.x + 14 + i * 25, y };
            if (!icon.render(text[i + startIndex], textPos, 6, { SSD1306_WHITE }, Icon::CENTER)) {
                draw.textCentered(textPos, text[i + startIndex], 6);
            }
        }
    }

public:
    void render()
    {
        renderRow(position.y, 0);
        renderRow(position.y + 8, 5);
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
