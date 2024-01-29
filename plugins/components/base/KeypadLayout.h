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
        uint16_t controllerId;
        uint8_t key;
        int param;
        std::function<void(int8_t state, KeyMap& keymap)> action;
        uint8_t color;
        std::function<uint8_t(KeyMap& keymap)> getColor;
    };

protected:
    // FIXME move keypad ptr to KeyMap
    KeypadInterface* keypad;
    std::function<void(uint16_t controllerId, int8_t state, int param, std::string action, uint8_t color)> addKeyMap;

public:
    std::vector<KeyMap> mapping;

    KeypadLayout(KeypadInterface* keypad, std::function<void(uint16_t controllerId, int8_t state, int param, std::string action, uint8_t color)> addKeyMap)
        : keypad(keypad)
        , addKeyMap(addKeyMap)
    {
    }

    void onKey(uint16_t id, int key, int8_t state)
    {
        for (KeyMap keyMap : mapping) {
            if (keyMap.controllerId == id && keyMap.key == key) {
                keyMap.action(state, keyMap);
                return;
            }
        }

        printf("unhandled key %d for controller %d\n", key, id);
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
            // FIXME move keypad ptr to KeyMap
            if (keyMap.controllerId == 0)
                continue;

            keypad->setButton(keyMap.key, keyMap.getColor(keyMap));
        }
    }

    bool config(char* key, char* value)
    {
        /*md - `KEYMAP: controllerName key action [param] [color]` Map an action to a controller key. Use `Keyboard` as `controllerName` to use computer keyboard. */
        if (strcmp(key, "KEYMAP") == 0) {
            std::string controllerName = strtok(value, " ");
            uint8_t key = atoi(strtok(NULL, " "));
            std::string action = strtok(NULL, " ");
            char* paramStr = strtok(NULL, " ");
            char* colorStr = strtok(NULL, " ");
            int param = paramStr != NULL ? atoi(paramStr) : -1;
            uint8_t color = colorStr != NULL ? atoi(colorStr) : 255;

            uint16_t controllerId = -1;
            if (strcmp(controllerName.c_str(), "Keyboard") == 0) {
                controllerId = 0;
            } else {
                // FIXME get id from list of controllers
                controllerId = keypad->id;
            }
            addKeyMap(controllerId, key, param, action, color);
        }
        return false;
    }
};

#endif