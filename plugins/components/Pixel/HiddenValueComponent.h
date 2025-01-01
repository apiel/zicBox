#ifndef _UI_COMPONENT_HIDDEN_VALUE_H_
#define _UI_COMPONENT_HIDDEN_VALUE_H_

#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"

#include <string>

/*md
## HiddenValue

Hidden value component is used to change a value without showing it.
*/
class HiddenValueComponent : public Component {
protected:
    int8_t encoderId = -1;

    ValueInterface* value = NULL;

    KeypadLayout keypadLayout;

public:
    HiddenValueComponent(ComponentInterface::Props props)
        : Component(props)
        , keypadLayout(this)
    {
    }

    void render()
    {
    }

    bool config(char* key, char* params)
    {
        if (keypadLayout.config(key, params)) {
            return true;
        }

        /*md - `VALUE: pluginName keyName` is used to set the value to control */
        if (strcmp(key, "VALUE") == 0) {
            char* pluginName = strtok(params, " ");
            char* keyValue = strtok(NULL, " ");
            value = getPlugin(pluginName, track).getValue(keyValue);
            return true;
        }

        /*md - `ENCODER_ID: 0` is used to set the encoder id that will interract with this component */
        if (strcmp(key, "ENCODER_ID") == 0) {
            encoderId = atoi(params);
            return true;
        }

        return false;
    }

    int8_t currentGroup = 0;
    void onGroupChanged(int8_t index) override
    {
        currentGroup = index;
    }

    void onEncoder(int id, int8_t direction) override
    {
        if (id == encoderId) {
            if (value) {
                value->increment(direction);
            } else { // By default it will change group
                view->setGroup(currentGroup + direction);
            }
        }
    }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        keypadLayout.onKey(id, key, state, now);
    }
};

#endif
