#ifndef _KEYPAD_LAYOUT_H
#define _KEYPAD_LAYOUT_H

#include <functional>
#include <stdint.h>
#include <string>
#include <string.h>
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
        KeypadInterface* controller;
        uint16_t controllerId;
        uint8_t key;
        int param;
        std::function<void(int8_t state, KeyMap& keymap)> action;
        uint8_t color;
        std::function<uint8_t(KeyMap& keymap)> getColor;
    };

protected:
    std::function<void(KeypadInterface* controller, uint16_t controllerId, int8_t state, int param, std::string action, uint8_t color)> addKeyMap;
    ControllerInterface* (*getController)(const char* name);

public:
    std::vector<KeyMap> mapping;

    KeypadLayout(ControllerInterface* (*getController)(const char* name), std::function<void(KeypadInterface* controller, uint16_t controllerId, int8_t state, int param, std::string action, uint8_t color)> addKeyMap)
        : getController(getController)
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

        // printf("unhandled key %d for controller %d\n", key, id);
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
        // TODO instead to do this should just set the one missing from the list...
        // setButton(254, 254); // set all button off
        for (KeyMap keyMap : mapping) {
            if (keyMap.controller && keyMap.controller->hasColor) {
                keyMap.controller->setButton(keyMap.key, keyMap.getColor(keyMap));
            }
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

            KeypadInterface* controller = NULL;
            uint16_t controllerId = -1;
            // printf("........search controller %s\n", controllerName.c_str());
            if (strcmp(controllerName.c_str(), "Keyboard") == 0) {
                controllerId = 0;
            } else {
                controller = (KeypadInterface *)getController(controllerName.c_str());
                if (controller == NULL) {
                    // printf("..................controller %s NOT fount\n", controllerName.c_str());
                    return true;
                }
                // printf("........controller %s id %d\n", controllerName.c_str(), controller->id);
                controllerId = controller->id;
            }
            addKeyMap(controller, controllerId, key, param, action, color);
            return true;
        }
        return false;
    }
};

#endif