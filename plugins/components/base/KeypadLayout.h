#pragma once

#include <functional>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>

#include "helpers/controller.h"
#include "log.h"
#include "plugins/components/componentInterface.h"
#include "plugins/controllers/keypadInterface.h"

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
        bool multipleKeyHandler = false;
        std::function<uint8_t(KeyMap& keymap)> getColor;
        bool isLongPress = false;
        unsigned long pressedTime = 0;
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
        bool multipleKeyHandler;
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
            props.useContext,
            props.contextId,
            props.contextValue,
            props.multipleKeyHandler,
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

    void init(nlohmann::json& config)
    {
        if (config.contains("keys") && config["keys"].is_array()) {
            for (auto& key : config["keys"]) {
                AddKeyMapProps props;
                if (key.contains("controller")) {
                    props.controller = (KeypadInterface*)component->getController(key["controller"].get<std::string>().c_str());
                    props.controllerId = props.controller->id;
                } else {
                    props.controllerId = 0; // Keyboard
                    props.controller = NULL;
                }
                if (!key.contains("key") || !key.contains("action")) {
                    logWarn("Keypad layout config is missing mandatory parameter");
                    continue;
                }

                if (key["key"].is_number_integer()) {
                    props.key = key["key"].get<uint8_t>();
                } else {
                    std::string keyStr = key["key"].get<std::string>();
                    if (keyStr.length() == 1) {
                        keyStr = "'" + keyStr + "'";
                    }
                    props.key = getKeyCode(keyStr.c_str());
                    // logDebug("Keystr %s key %d", keyStr.c_str(), props.key);
                }

                props.action = key["action"].get<std::string>();
                if (key.contains("action2")) {
                    props.action2 = key["action2"].get<std::string>();
                }
                // if (key.contains("actionLongPress")) {
                //     props.actionLongPress = key["actionLongPress"].get<std::string>();
                // }

                props.useContext = false;
                if (key.contains("context")) {
                    props.useContext = true;
                    props.contextId = key["context"]["id"].get<uint8_t>();
                    props.contextValue = key["context"]["value"].get<float>();
                }

                props.multipleKeyHandler = key.value("multipleKeyHandler", false);

                // logDebug("Adding keymap: controller %d key %d action %s", props.controllerId, props.key, props.action.c_str());

                addKeyMap(props);
            }
        }
    }

    static bool isPressed(KeyMap& keyMap)
    {
        return keyMap.pressedTime != 0;
    }

    static bool isReleased(KeyMap& keyMap)
    {
        return keyMap.pressedTime == 0;
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

    bool onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        // printf("keypad id %d key %d state %d mapping.size %ld\n", id, key, state, mapping.size());
        for (KeyMap& keyMap : mapping) {
            if (keyMap.controllerId == id && keyMap.key == key
                && (!keyMap.useContext || component->view->contextVar[keyMap.contextId] == keyMap.contextValue)) {
                bool actionDone = false;
                if (state == 1) {
                    // keyMap.isLongPress = false;
                    // To know if the key is pressed, we set pressedTime to the current time
                    // It also allow us to calculate the duration of the key press for long press
                    keyMap.pressedTime = now;
                } else {
                    // To know if the key is not pressed anymore, we set pressedTime to 0
                    keyMap.pressedTime = 0;
                    // if (keyMap.isLongPress && keyMap.actionLongPress) {
                    //     keyMap.actionLongPress(keyMap);
                    //     renderKeypadColor(keyMap);
                    // }
                }
                if (keyMap.action) {
                    // printf("--> call action keymap id %d key %d\n", keyMap.controllerId, keyMap.key);
                    keyMap.action(keyMap);
                    renderKeypadColor(keyMap);
                    actionDone = true;
                }
                if (keyMap.action2) {
                    keyMap.action2(keyMap);
                    renderKeypadColor(keyMap);
                    actionDone = true;
                }
                return !keyMap.multipleKeyHandler && actionDone;
            }
        }
        return false;
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

        if (action.rfind("sh:") == 0) {
            std::string* paramFn = new std::string(action.substr(3));
            return [this, paramFn](KeypadLayout::KeyMap& keymap) {
                if (isReleased(keymap)) {
                    system(paramFn->c_str());
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

        if (action.rfind("setContext:") == 0) {
            const char* params = action.substr(11).c_str();
            uint8_t* indexVar = new uint8_t(atoi(strtok((char*)params, ":")));
            uint8_t* value = new uint8_t(atoi(strtok(NULL, ":")));
            return [this, indexVar, value](KeypadLayout::KeyMap& keymap) {
                component->setContext(*indexVar, *value);
            };
        }

        if (action.rfind("contextToggle:") == 0) {
            const char* params = action.substr(14).c_str();
            uint8_t* indexVar = new uint8_t(atoi(strtok((char*)params, ":")));
            uint8_t* shiftPressed = new uint8_t(atoi(strtok(NULL, ":")));
            uint8_t* shiftReleased = new uint8_t(atoi(strtok(NULL, ":")));
            return [this, indexVar, shiftPressed, shiftReleased](KeypadLayout::KeyMap& keymap) {
                // printf("contextToggle %d %d %d %f\n", *indexVar, *shiftPressed, *shiftReleased, component->view->contextVar[*indexVar]);
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
                    // printf("contextToggleOnRelease %d %d %d %f\n", *indexVar, *shiftA, *shiftB, component->view->contextVar[*indexVar]);
                    component->setContext(*indexVar, component->view->contextVar[*indexVar] == *shiftA ? *shiftB : *shiftA);
                    // printf("result: %f\n", component->view->contextVar[*indexVar]);
                }
            };
        }

        if (action.rfind("audioEvent:") == 0) {
            AudioEventType id = getEventTypeFromName(action.substr(11).c_str());
            if (id == AudioEventType::UNKNOWN) {
                logWarn("[Keypad] Unknown audio event: %s", action.substr(11).c_str());
                return NULL;
            }
            return [this, id](KeypadLayout::KeyMap& keymap) {
                if (isReleased(keymap)) {
                    component->sendAudioEvent(id, -1);
                }
            };
        }

        if (action == "playPause") {
            return [this](KeypadLayout::KeyMap& keymap) {
                if (isReleased(keymap)) {
                    component->sendAudioEvent(AudioEventType::TOGGLE_PLAY_PAUSE, -1);
                }
                // long press could trigger stop
            };
        }

        if (action == "stop") {
            return [this](KeypadLayout::KeyMap& keymap) {
                if (isReleased(keymap)) {
                    component->sendAudioEvent(AudioEventType::STOP, -1);
                }
                // long press could trigger stop
            };
        }

        if (action.rfind("setActiveTrack:") == 0) {
            int trackId = atoi(action.substr(15).c_str());
            return [this, trackId](KeypadLayout::KeyMap& keymap) {
                if (isReleased(keymap)) {
                    component->sendAudioEvent(AudioEventType::SET_ACTIVE_TRACK, trackId);
                }
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
            if (plugin && noteStr != NULL) {
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

        if (action.rfind("noteRepeat:") == 0) {
            std::string substring = action.substr(11);
            std::vector<char> params(substring.begin(), substring.end());
            params.push_back('\0');

            char* pluginName = strtok(params.data(), ":");
            char* noteStr = strtok(NULL, ":");
            char* modeStr = strtok(NULL, ":");
            char* trackStr = strtok(NULL, ":");
            AudioPlugin* plugin = &component->getPlugin(pluginName, trackStr != NULL ? atoi(trackStr) : component->track);
            if (plugin && modeStr != NULL && noteStr != NULL) {
                uint8_t* note = new uint8_t(atoi(noteStr));
                uint8_t* mode = new uint8_t(atoi(modeStr));
                return [this, plugin, note, mode](KeypadLayout::KeyMap& keymap) {
                    if (isPressed(keymap)) {
                        plugin->noteRepeatOn(*note, *mode);
                    } else {
                        plugin->noteRepeatOff(*note);
                    }
                };
            }
        }

        if (action.rfind("noteOnAndRepeat:") == 0) {
            std::string substring = action.substr(16);
            std::vector<char> params(substring.begin(), substring.end());
            params.push_back('\0');

            char* pluginName = strtok(params.data(), ":");
            char* noteStr = strtok(NULL, ":");
            char* modeStr = strtok(NULL, ":");
            char* trackStr = strtok(NULL, ":");

            // Hardcoded to sequencer for the moment
            AudioPlugin* seqPlugin = &component->getPlugin("Sequencer", trackStr != NULL ? atoi(trackStr) : component->track);
            AudioPlugin* synthPlugin = &component->getPlugin(pluginName, trackStr != NULL ? atoi(trackStr) : component->track);
            if (synthPlugin && modeStr != NULL && noteStr != NULL) {
                // Hardcoded as well
                bool* seqPlayingPtr = (bool*)seqPlugin->data(seqPlugin->getDataId("IS_PLAYING"));
                uint8_t* note = new uint8_t(atoi(noteStr));
                uint8_t* mode = new uint8_t(modeStr == "val" ? 0 : atoi(modeStr));
                return [this, seqPlugin, seqPlayingPtr, synthPlugin, note, mode](KeypadLayout::KeyMap& keymap) {
                    if (seqPlugin && *seqPlayingPtr) {
                        if (isPressed(keymap)) {
                            seqPlugin->noteRepeatOn(*note, *mode);
                        } else {
                            seqPlugin->noteRepeatOff(*note);
                        }
                    } else {
                        if (isPressed(keymap)) {
                            synthPlugin->noteOn(*note, 1.0f);
                        } else {
                            synthPlugin->noteOff(*note, 0.0f);
                        }
                    }
                };
            }
        }

        if (action.rfind("debug:") == 0) {
            std::string substring = action.substr(6);
            std::vector<char> text(substring.begin(), substring.end());
            text.push_back('\0');

            return [this, text](KeypadLayout::KeyMap& keymap) {
                if (isReleased(keymap)) {
                    printf(">>> keypad debug: %s\n", text.data());
                }
            };
        }

        return NULL;
    }

    std::function<uint8_t(KeypadLayout::KeyMap& keymap)>
    getColorAction(std::string action)
    {
        return NULL;
    }
};