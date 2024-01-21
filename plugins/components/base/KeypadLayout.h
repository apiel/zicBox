#ifndef _KEYPAD_LAYOUT_H
#define _KEYPAD_LAYOUT_H

#include <functional>
#include <stdint.h>
#include <string>
#include <vector>

class KeypadLayout {
public:
    std::string name = "default";

    struct KeyMap {
        uint8_t key;
        int param;
        std::function<uint8_t(int param)> color;
        std::function<void(int8_t state, int param)> action;
    };

protected:
    void (*setButton)(int id, uint8_t color);
    std::vector<KeyMap> mapping;

    void onKeypad(int key, int8_t state)
    {
        for (KeyMap keyMap : mapping) {
            if (keyMap.key == key) {
                keyMap.action(state, keyMap.param);
                return;
            }
        }

        printf("unhandled key %d\n", key);
    }

public:
    KeypadLayout(void (*setButton)(int id, uint8_t color))
        : setButton(setButton)
    {
    }

    // Might want to render keypad in a central place
    // However, how to switch of layout in the same view?
    void renderKeypad()
    {
        // TODO instead to do this should just set the one missing from the list...
        // setButton(254, 254); // set all button off
        for (KeyMap keyMap : mapping) {
            setButton(keyMap.key, keyMap.color(keyMap.param));
        }
    }

    bool config(char* key, char* value)
    {
        // /*md - `KEYMAP: pluginName action param` set default keymap */
        // if (strcmp(key, "KEYMAP") == 0) {
        //     char* pluginName = strtok(value, " ");
        //     char* action = strtok(NULL, " ");
        //     int param = atoi(strtok(NULL, " "));
        //     // value = val(getPlugin(pluginName, track).getValue(keyValue));
        // }
        return false;
    }
};

#endif