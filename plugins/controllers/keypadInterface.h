#ifndef _KEYPAD_INTERFACE_H_
#define _KEYPAD_INTERFACE_H_

#include "controllerInterface.h"

// #include <functional>

/*md
## KeypadInterface

Keypad interface define how to handle keypad events.
*/
class KeypadInterface : public ControllerInterface {
// public:
//     struct KeyMap {
//         uint8_t key;
//         int param;
//         std::function<uint8_t(int param)> color;
//         std::function<void(int8_t state, int param)> action;
//     };

//     struct KeyMapLayout {
//         std::string name;
//         std::vector<KeyMap> mapping;
//     };

// protected:
//     std::vector<KeyMapLayout> layouts = {
//         { "default", {} }
//     };
//     KeyMapLayout& activeLayout;

//     void onKeypad(int key, int8_t state)
//     {
//         for (KeyMap keyMap : activeLayout.mapping) {
//             if (keyMap.key == key) {
//                 keyMap.action(state, keyMap.param);
//                 return;
//             }
//         }

//         printf("unhandled key %d\n", key);
//     }

public:
    KeypadInterface(ControllerInterface::Props& props)
        : ControllerInterface(props)
        // , activeLayout(layouts[0])
    {
    }

    virtual void setButton(int id, uint8_t color) = 0;

    // void renderKeypad()
    // {
    //     // TODO instead to do this should just set the one missing from the list...
    //     setButton(254, 254); // set all button off

    //     for (KeyMap keyMap : activeLayout.mapping) {
    //         setButton(keyMap.key, keyMap.color(keyMap.param));
    //     }
    // }

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