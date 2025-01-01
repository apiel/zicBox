#ifndef _KEYPAD_LAYOUT_H
#define _KEYPAD_LAYOUT_H

#include <functional>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>

#include "helpers/controller.h"
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
        std::function<void(KeyMap& keymap)> action;
        std::function<void(KeyMap& keymap)> action2;
        // std::function<void(KeyMap& keymap)> actionLongPress;
        bool useContext = false;
        uint8_t contextId = 0;
        float contextValue = 0.0f;
        std::function<uint8_t(KeyMap& keymap)> getColor;
        bool isLongPress = false;
        unsigned long pressedTime = -1;
    };

    struct AddKeyMapProps {
        KeypadInterface* controller;
        uint16_t controllerId;
        uint8_t key;
        std::string action;
        std::string action2;
        // std::string actionLongPress;
        bool useContext;
        uint8_t contextId;
        float contextValue;
    };

    ComponentInterface* component;

    std::vector<KeyMap> mapping;

    std::function<std::function<void(KeypadLayout::KeyMap& keymap)>(std::string action)> getCustomAction;

    // Publicly available to be eventually overridden
    std::function<void(AddKeyMapProps props)> addKeyMap = [this](AddKeyMapProps props) {
        std::function<void(KeypadLayout::KeyMap & keymap)> actionFn = getAction(props.action);
        if (!actionFn && getCustomAction) {
            actionFn = getCustomAction(props.action);
        }

        std::function<void(KeypadLayout::KeyMap & keymap)> action2Fn = getAction(props.action2);
        if (!action2Fn && getCustomAction) {
            action2Fn = getCustomAction(props.action2);
        }

        // std::function<void(KeypadLayout::KeyMap & keymap)> actionLongPressFn = getAction(props.actionLongPress);
        // if (!actionLongPressFn) {
        //     actionLongPressFn = getCustomAction(props.actionLongPress);
        // }

        // TODO handle keypad color
        // color should be define base on the state of keymap
        // using pressedTime to know if the key is pressed
        // and using isLongPress to know if the key is long pressed
        // --> should there be a getColor for each mode short press / long press ?
        // --> should get color be define base on action name ?
        // --> couldn't we instead have an array of color for each mode (short pressed, long pressed, short released, long released) ?
        // In the current version, we are not using keypad color, so let's postpone those decisions
        std::function<uint8_t(KeyMap & keymap)> getColorFn = getColorAction(props.action);
        if (!getColorFn) {
            // getColorFn = getCustomColorAction(props.color);
        }

        mapping.push_back({
            props.controller,
            props.controllerId,
            props.key,
            actionFn,
            action2Fn,
            // actionLongPressFn,
        });
    };

    KeypadLayout(ComponentInterface* component)
        : component(component)
    {
    }

    KeypadLayout(ComponentInterface* component, std::function<std::function<void(KeypadLayout::KeyMap& keymap)>(std::string action)> getCustomAction)
        : component(component)
        , getCustomAction(getCustomAction)
    {
    }

    static bool isPressed(KeyMap& keyMap)
    {
        return keyMap.pressedTime != -1;
    }

    static bool isReleased(KeyMap& keyMap)
    {
        return keyMap.pressedTime == -1;
    }

    // To be used if longPress action is activated
    // in component constructor do: jobRendering = [this](unsigned long now) { if (keypadLayout.jobRendering(now)) { renderNext(); } };
    //
    // bool jobRendering(unsigned long now)
    // {
    //     for (KeyMap& keyMap : mapping) {
    //         if (keyMap.actionLongPress && !keyMap.isLongPress && isPressed(keyMap) && now - keyMap.pressedTime > 500) {
    //             keyMap.isLongPress = true;
    //             keyMap.actionLongPress(keyMap);
    //             renderKeypadColor(keyMap);
    //             return true;
    //         }
    //     }
    //     return false;
    // }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        // printf("keypad id %d key %d state %d\n", id, key, state);
        for (KeyMap& keyMap : mapping) {
            if (keyMap.controllerId == id && keyMap.key == key
                && (!keyMap.useContext || component->view->contextVar[keyMap.contextId] == keyMap.contextValue)) {
                if (state == 1) {
                    // keyMap.isLongPress = false;
                    // To know if the key is pressed, we set pressedTime to the current time
                    // It also allow us to calculate the duration of the key press for long press
                    keyMap.pressedTime = now;
                } else {
                    // To know if the key is not pressed anymore, we set pressedTime to -1
                    keyMap.pressedTime = -1;
                    // if (keyMap.isLongPress && keyMap.actionLongPress) {
                    //     keyMap.actionLongPress(keyMap);
                    //     renderKeypadColor(keyMap);
                    // }
                }
                if (keyMap.action) {
                    // printf("--> call action keymap id %d key %d\n", keyMap.controllerId, keyMap.key);
                    keyMap.action(keyMap);
                    renderKeypadColor(keyMap);
                }
                if (keyMap.action2) {
                    keyMap.action2(keyMap);
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

    std::function<void(KeypadLayout::KeyMap& keymap)> getAction(std::string action)
    {
        if (action.rfind("setView:") == 0) {
            std::string* paramFn = new std::string(action.substr(8));
            return [this, paramFn](KeypadLayout::KeyMap& keymap) {
                if (isReleased(keymap)) {
                    component->view->setView(*paramFn);
                }
            };
        }

        if (action.rfind("incGroup:") == 0) {
            int direction = action[9] == '-' ? -1 : 1;
            int incValue = atoi(action.substr(10).c_str());
            int* paramFn = new int(incValue * direction);
            return [this, paramFn](KeypadLayout::KeyMap& keymap) {
                if (isReleased(keymap)) {
                    component->view->setGroup(component->view->activeGroup + *paramFn);
                }
            };
        }

        if (action.rfind("setGroup:") == 0) {
            int* paramFn = new int(atoi(action.substr(9).c_str()));
            return [this, paramFn](KeypadLayout::KeyMap& keymap) {
                if (isReleased(keymap)) {
                    component->view->setGroup(*paramFn);
                }
            };
        }

        if (action.rfind("contextToggle:") == 0) {
            const char* params = action.substr(14).c_str();
            uint8_t* indexVar = new uint8_t(atoi(strtok((char*)params, ":")));
            uint8_t* shiftPressed = new uint8_t(atoi(strtok(NULL, ":")));
            uint8_t* shiftReleased = new uint8_t(atoi(strtok(NULL, ":")));
            return [this, indexVar, shiftPressed, shiftReleased](KeypadLayout::KeyMap& keymap) {
                component->setContext(*indexVar, isReleased(keymap) ? *shiftReleased : *shiftPressed);
            };
        }

        // Unlike contextToggle, it will only toggle on release
        if (action.rfind("contextToggleOnRelease:") == 0) {
            const char* params = action.substr(23).c_str();
            uint8_t* indexVar = new uint8_t(atoi(strtok((char*)params, ":")));
            uint8_t* shiftA = new uint8_t(atoi(strtok(NULL, ":")));
            uint8_t* shiftB = new uint8_t(atoi(strtok(NULL, ":")));
            return [this, indexVar, shiftA, shiftB](KeypadLayout::KeyMap& keymap) {
                if (isReleased(keymap)) {
                    component->setContext(*indexVar, component->view->contextVar[*indexVar] == *shiftA ? *shiftB : *shiftA);
                }
            };
        }

        if (action == "playPause") {
            return [this](KeypadLayout::KeyMap& keymap) {
                if (isReleased(keymap)) {
                    component->sendAudioEvent(AudioEventType::TOGGLE_PLAY_PAUSE);
                }
                // long press could trigger stop
            };
        }

        if (action.rfind("noteOn:") == 0) {
            std::string substring = action.substr(7);
            std::vector<char> params(substring.begin(), substring.end());
            params.push_back('\0');

            char* pluginName = strtok(params.data(), ":");
            char* noteStr = strtok(NULL, ":");
            char* trackStr = strtok(NULL, ":");
            AudioPlugin* plugin = &component->getPlugin(pluginName, trackStr != NULL ? atoi(trackStr) : component->track);
            if (plugin) {
                uint8_t* note = new uint8_t(atoi(noteStr));
                return [this, plugin, note](KeypadLayout::KeyMap& keymap) {
                    if (isPressed(keymap)) {
                        plugin->noteOn(*note, 1.0f);
                    } else {
                        plugin->noteOff(*note, 0.0f);
                    }
                };
            }
        }

        return NULL;
    }

    std::function<uint8_t(KeypadLayout::KeyMap& keymap)>
    getColorAction(std::string action)
    {
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

            bool useContext = false;
            uint8_t contextId = 0;
            float contextValue = 0.0f;
            size_t pos = controllerName.find(":");
            if (pos != std::string::npos) {
                std::string contextStr = controllerName.c_str() + pos + 1;
                controllerName = controllerName.substr(0, pos);
                contextId = atoi(strtok((char*)contextStr.c_str(), ":"));
                contextValue = atof(strtok(NULL, ":"));
                useContext = true;
            }

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
            // printf("add keymap %d %d action: %s longpress: %s\n", controllerId, key, action.c_str(), actionLongPress.c_str());
            addKeyMap({ controller, controllerId, key, action, actionLongPress, useContext, contextId, contextValue });
            return true;
        }
        return false;
    }
};

#endif