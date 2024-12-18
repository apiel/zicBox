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
    KeyInfoBarComponent(ComponentInterface::Props props)
        : Component(props)
        , icon(props.view->draw)
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action.rfind("item:") == 0) {
                int* paramFn = new int(atoi(action.substr(5).c_str()));
                func = [this, paramFn](KeypadLayout::KeyMap& keymap) { handleButton(*paramFn, keymap.pressedTime != -1); };
            }
            return func;
        })
    {
    }

protected:
    std::string text[10] = {
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

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        keypadLayout.onKey(id, key, state, now);
    }

    void initView(uint16_t counter)
    {
        keypadLayout.renderKeypadColor();
    }
};

#endif
