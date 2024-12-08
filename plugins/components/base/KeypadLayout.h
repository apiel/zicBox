#ifndef _KEYPAD_LAYOUT_H
#define _KEYPAD_LAYOUT_H

#include <functional>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>

#include "log.h"
#include "plugins/components/componentInterface.h"
#include "plugins/controllers/keypadInterface.h"

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
        std::function<void(int8_t state, KeyMap& keymap)> action;
        void* param = NULL;
        std::function<void(int8_t state, KeyMap& keymap)> actionLongPress = [](int8_t state, KeyMap& keymap) {};
        void* paramLongPress = NULL;
        uint8_t color = 255;
        std::function<uint8_t(KeyMap& keymap)> getColor = [&](KeyMap& keymap) { return keymap.color; };
        bool isLongPress = false;
        unsigned long pressedTime = -1;
    };

protected:
    ComponentInterface::Props& componentProps;
    std::function<void(KeypadInterface* controller, uint16_t controllerId, int8_t state, std::string action, char* param, std::string actionLongPress, char* paramLongPress)> addKeyMap;
    ControllerInterface* (*getController)(const char* name);

    uint8_t getKeyCode(std::string keyStr)
    {
        if (keyStr[0] == '\'' && keyStr[2] == '\'') {
            // A is 4 and Z is 29
            if (keyStr[1] >= 97 && keyStr[1] <= 123) {
                return keyStr[1] - 97 + 4;
            }

            // a is 4 and z is 29
            if (keyStr[1] >= 65 && keyStr[1] <= 90) {
                return keyStr[1] - 65 + 4;
            }

            // 0 is 30 and 9 is 39
            if (keyStr[1] >= 48 && keyStr[1] <= 57) {
                return keyStr[1] - 48 + 30;
            }

            logWarn("Unsupported key: " + keyStr + ". Supported keys: a-z, A-Z, 0-9. For other special char, use scancode values.");
            return -1; // 255
        }

        return atoi(keyStr.c_str());
    }

public:
    std::vector<KeyMap> mapping;

    KeypadLayout(ComponentInterface::Props& props, ControllerInterface* (*getController)(const char* name), std::function<void(KeypadInterface* controller, uint16_t controllerId, int8_t state, std::string action, char* param, std::string actionLongPress, char* paramLongPress)> addKeyMap)
        : componentProps(props)
        , getController(getController)
        , addKeyMap(addKeyMap)
    {
    }

    bool jobRendering(unsigned long now)
    {
        for (KeyMap& keyMap : mapping) {
            if (!keyMap.isLongPress && keyMap.pressedTime != -1 && now - keyMap.pressedTime > 500) {
                keyMap.isLongPress = true;
                keyMap.actionLongPress(1, keyMap);
                return true;
            }
        }
        return false;
    }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        for (KeyMap& keyMap : mapping) {
            if (keyMap.controllerId == id && keyMap.key == key) {
                if (state == 1) {
                    keyMap.isLongPress = false;
                    keyMap.pressedTime = now;
                } else if (keyMap.isLongPress) {
                    keyMap.actionLongPress(0, keyMap);
                }
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

    bool setAction(std::string action, void* param, std::function<void(int8_t state, KeyMap& keymap)>& actionFn, void* paramFn)
    {
        if (action == "setView") {
            paramFn = new std::string((char*)param);
            actionFn = [this](int8_t state, KeyMap& keymap) { componentProps.view->setView(*(std::string*)keymap.param); };
            return true;
        }
        // if (action == "setGroup") {
        //     paramFn = new int(atoi((char*)param));
        //     actionFn = [this](int8_t state, KeyMap& keymap) { componentProps.view->setGroup(*(int*)keymap.param); };
        //     return true;
        // }
        return false;
    }

    bool config(char* key, char* value)
    {
        /*md - `KEYMAP: controllerName key action [param] [color]` Map an action to a controller key. Use `Keyboard` as `controllerName` to use computer keyboard. Use 'A' to 'Z' and '0' to '9' to use computer keyboard. Other key should use scancode number without quotes. */
        if (strcmp(key, "KEYMAP") == 0) {
            std::string controllerName = strtok(value, " ");
            uint8_t key = getKeyCode(strtok(NULL, " "));

            char* actionPtr = strtok(NULL, " ");
            char* actionLongPressPtr = strtok(NULL, " ");

            std::string action = strtok(actionPtr, ":");
            char* param = strtok(NULL, ":");

            std::string actionLongPress = actionLongPressPtr ? strtok(actionLongPressPtr, ":") : "";
            char* paramLongPress = actionLongPressPtr ? strtok(NULL, ":") : NULL;

            KeypadInterface* controller = NULL;
            uint16_t controllerId = -1;
            // printf("........search controller %s\n", controllerName.c_str());
            if (strcmp(controllerName.c_str(), "Keyboard") == 0) {
                controllerId = 0;
            } else {
                controller = (KeypadInterface*)getController(controllerName.c_str());
                if (controller == NULL) {
                    // printf("..................controller %s NOT fount\n", controllerName.c_str());
                    return true;
                }
                // printf("........controller %s id %d\n", controllerName.c_str(), controller->id);
                controllerId = controller->id;
            }
            addKeyMap(controller, controllerId, key, action, param, actionLongPress, paramLongPress);
            return true;
        }
        return false;
    }
};

#endif