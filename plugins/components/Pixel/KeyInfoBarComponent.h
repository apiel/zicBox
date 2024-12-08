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

// Instead to have child elements, use same concept as group...
// presskey can set visibility of a given component

// KEYMAP in addition of key number, we could use engish keyboard value

class KeyInfoBarComponent : public Component {
protected:
    Icon icon;
    KeypadLayout keypadLayout;
    int8_t activeGroup = 0;

    Color textColor;

    //                        { index, value}
    int16_t shiftVisibility[2] = { -1, -1 };

    uint8_t shiftIndex = 255;

    void handleButton(int8_t id, int8_t state)
    {
        printf("handleButton: %d %d\n", id, state);

        if (state == 1) {
            if (id == 0) {
                view->setGroup(activeGroup - 1);
            } else if (id == 1) {
                view->setView("Sequencer");
            } else if (id == 4) {
                setShift(shiftIndex, view->shift[shiftIndex] ? 0 : 1);
            } else if (id == 5) {
                view->setGroup(activeGroup + 1);
            } else if (id == 9) {
                sendAudioEvent(AudioEventType::TOGGLE_PLAY_PAUSE);
            } else {
                AudioPlugin* plugin = &getPlugin("SynthDrum23", 1);
                plugin->noteOn(60, 1.0f);
            }
        }
    }

public:
    void addKeyMap(KeypadInterface* controller, uint16_t controllerId, uint8_t key, std::string action, char* param, std::string actionLongPress, char* paramLongPress)
    {
        if (action == "item") {
            keypadLayout.mapping.push_back(
                {
                    controller,
                    controllerId,
                    key,
                    [&](int8_t state, KeypadLayout::KeyMap& keymap) { handleButton(*(int*)keymap.param, state); },
                    new int(atoi(param)),
                    [&](int8_t state, KeypadLayout::KeyMap& keymap) { printf("longpress test\n"); },
                });
        }
    }

    KeyInfoBarComponent(ComponentInterface::Props props)
        : Component(props)
        , icon(props.view->draw)
        , keypadLayout(getController, [&](KeypadInterface* controller, uint16_t controllerId, int8_t key, std::string action, char* param, std::string actionLongPress, char* paramLongPress) { addKeyMap(controller, controllerId, key, action, param, actionLongPress, paramLongPress); })
        , textColor(styles.colors.text)
    {
        buttonWidth = size.w / 5.0f;
        buttonStartX = buttonWidth * 0.5f;
    }

protected:
    int buttonWidth = 40;
    int buttonStartX = 14;

    std::string text[10] = { "", "", "", "", "", "", "", "", "", "" };
    void renderRow(int y, int startIndex)
    {
        for (int i = 0; i < 5; i++) {
            Point textPos = { relativePosition.x + buttonStartX + i * buttonWidth, y };
            if (!icon.render(text[i + startIndex], textPos, 8, {}, Icon::CENTER)) {
                draw.textCentered(textPos, text[i + startIndex], 8);
            }
        }
    }

    bool isVisible()
    {
        return shiftVisibility[0] == -1 || view->shift[shiftVisibility[0]] == shiftVisibility[1];
    }

public:
    void render()
    {
        if (isVisible() && updatePosition()) {
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

        /*md - `ROW1: text text text text text` is the text of the first row. */
        if (strcmp(key, "ROW1") == 0) {
            text[0] = strtok(value, " ");
            text[1] = strtok(NULL, " ");
            text[2] = strtok(NULL, " ");
            text[3] = strtok(NULL, " ");
            text[4] = strtok(NULL, " ");
            return true;
        }

        /*md - `ROW2: text text text text text` is the text of the second row. */
        if (strcmp(key, "ROW2") == 0) {
            text[5] = strtok(value, " ");
            text[6] = strtok(NULL, " ");
            text[7] = strtok(NULL, " ");
            text[8] = strtok(NULL, " ");
            text[9] = strtok(NULL, " ");
            return true;
        }

        /*md - `SHIFT_VISIBILITY: index value` is the index and value to make the component visible or not. */
        if (strcmp(key, "SHIFT_VISIBILITY") == 0) {
            shiftVisibility[0] = atoi(strtok(value, " "));
            shiftVisibility[1] = atoi(strtok(NULL, " "));
            return true;
        }

        /*md - `TEXT_COLOR: color` is the color of the text. */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            textColor = draw.getColor(value);
            return true;
        }

        return false;
    }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        if (isVisible()) {
            keypadLayout.onKey(id, key, state, now);
        }
    }

    void onGroupChanged(int8_t index) override
    {
        activeGroup = index;
    }

    void initView(uint16_t counter)
    {
        keypadLayout.renderKeypad();
    }
};

#endif
