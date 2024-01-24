#ifndef _KEYPAD_LAYOUT_H
#define _KEYPAD_LAYOUT_H

#include <functional>
#include <stdint.h>
#include <string>
#include <vector>

#include "../../controllers/keypadInterface.h"

/*md
## KeypadLayout

Some components might want to use a keypad layout.

*/
class KeypadLayout {
public:
    std::string name = "default";

    struct KeyMap {
        uint8_t key;
        int param;
        uint8_t color;
        std::function<uint8_t(KeyMap& keymap)> getColor;
        std::function<void(int8_t state, KeyMap& keymap)> action;
    };

protected:
    KeypadInterface* keypad;
    std::function<void(int8_t state, int param, std::string action, uint8_t color)> addKeyMap;

public:
    std::vector<KeyMap> mapping;

    KeypadLayout(KeypadInterface* keypad, std::function<void(int8_t state, int param, std::string action, uint8_t color)> addKeyMap)
        : keypad(keypad)
        , addKeyMap(addKeyMap)
    {
    }

    void onKeypad(int key, int8_t state)
    {
        for (KeyMap keyMap : mapping) {
            if (keyMap.key == key) {
                keyMap.action(state, keyMap);
                return;
            }
        }

        printf("unhandled key %d\n", key);
    }

    // Might want to render keypad in a central place
    // However, how to switch of layout in the same view?
    // void renderKeypad(void (*setButton)(int id, uint8_t color))
    // void renderKeypad(std::function<void(int id, uint8_t color)> setButton)
    // {
    //     // TODO instead to do this should just set the one missing from the list...
    //     // setButton(254, 254); // set all button off
    //     for (KeyMap keyMap : mapping) {
    //         setButton(keyMap.key, keyMap.color(keyMap.param));
    //     }
    // }
    void renderKeypad()
    {
        if (!keypad) {
            return;
        }

        // TODO instead to do this should just set the one missing from the list...
        // setButton(254, 254); // set all button off
        for (KeyMap keyMap : mapping) {
            keypad->setButton(keyMap.key, keyMap.getColor(keyMap));
        }
    }

    bool config(char* key, char* value)
    {
        /*md - `KEYMAP: key action [param] [color]` */
        if (strcmp(key, "KEYMAP") == 0) {
            uint8_t key = atoi(strtok(value, " "));
            std::string action = strtok(NULL, " ");
            char* paramStr = strtok(NULL, " ");
            char* colorStr = strtok(NULL, " ");
            int param = paramStr != NULL ? atoi(paramStr) : -1;
            uint8_t color = colorStr != NULL ? atoi(colorStr) : 255;
            addKeyMap(key, param, action, color);
        }
        return false;
    }
};

#endif