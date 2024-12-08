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
        std::function<void(int8_t state, KeyMap& keymap)> actionLongPress;
        std::function<uint8_t(KeyMap& keymap)> getColor;
        bool isLongPress = false;
        unsigned long pressedTime = -1;
    };

    struct AddKeyMapProps {
        KeypadInterface* controller;
        uint16_t controllerId;
        uint8_t key;
        std::string action;
        std::string actionLongPress;
    };

protected:
    std::function<void(AddKeyMapProps props)> addKeyMap;
    ComponentInterface* component;

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

    KeypadLayout(ComponentInterface* component, std::function<void(AddKeyMapProps props)> addKeyMap)
        : component(component)
        , addKeyMap(addKeyMap)
    {
    }

    bool jobRendering(unsigned long now)
    {
        for (KeyMap& keyMap : mapping) {
            if (keyMap.actionLongPress && !keyMap.isLongPress && keyMap.pressedTime != -1 && now - keyMap.pressedTime > 500) {
                keyMap.isLongPress = true;
                keyMap.actionLongPress(1, keyMap);
                renderKeypadColor(keyMap);
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
                } else {
                    keyMap.pressedTime = -1;
                    if (keyMap.isLongPress && keyMap.actionLongPress) {
                        keyMap.actionLongPress(0, keyMap);
                        renderKeypadColor(keyMap);
                    }
                }
                if (keyMap.action) {
                    keyMap.action(state, keyMap);
                    renderKeypadColor(keyMap);
                }
                return;
            }
        }
    }

    void renderKeypadColor()
    {
        // TODO instead to do this should just set the one missing from the list...
        // setButton(254, 254); // set all button off
        for (KeyMap keyMap : mapping) {
            renderKeypadColor(keyMap);
        }
    }

    void renderKeypadColor(KeyMap keyMap)
    {
        if (keyMap.getColor && keyMap.controller && keyMap.controller->hasColor) {
            keyMap.controller->setButton(keyMap.key, keyMap.getColor(keyMap));
        }
    }

    std::function<void(int8_t state, KeypadLayout::KeyMap& keymap)> getAction(std::string action)
    {
        if (action.rfind("setView:") == 0) {
            std::string* paramFn = new std::string(action.substr(8));
            return [this, paramFn](int8_t state, KeypadLayout::KeyMap& keymap) {
                component->view->setView(*paramFn);
            };
        }
        return NULL;
    }

    bool config(char* key, char* value)
    {
        /*md - `KEYMAP: controllerName key action [param] [color]` Map an action to a controller key. Use `Keyboard` as `controllerName` to use computer keyboard. Use 'A' to 'Z' and '0' to '9' to use computer keyboard. Other key should use scancode number without quotes. */
        if (strcmp(key, "KEYMAP") == 0) {
            std::string controllerName = strtok(value, " ");
            uint8_t key = getKeyCode(strtok(NULL, " "));

            std::string action = strtok(NULL, " ");
            char* actionLongPressPtr = strtok(NULL, " ");
            std::string actionLongPress = actionLongPressPtr ? actionLongPressPtr : "";

            KeypadInterface* controller = NULL;
            uint16_t controllerId = -1;
            // printf("........search controller %s\n", controllerName.c_str());
            if (strcmp(controllerName.c_str(), "Keyboard") == 0) {
                controllerId = 0;
            } else {
                controller = (KeypadInterface*)component->getController(controllerName.c_str());
                if (controller == NULL) {
                    // printf("..................controller %s NOT fount\n", controllerName.c_str());
                    return true;
                }
                // printf("........controller %s id %d\n", controllerName.c_str(), controller->id);
                controllerId = controller->id;
            }
            addKeyMap({ controller, controllerId, key, action, actionLongPress });
            return true;
        }
        return false;
    }
};

#endif