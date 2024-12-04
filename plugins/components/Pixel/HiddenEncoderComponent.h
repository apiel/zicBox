#ifndef _UI_COMPONENT_HIDDEN_ENCODER_H_
#define _UI_COMPONENT_HIDDEN_ENCODER_H_

#include "../component.h"
#include <string>

/*md
## HiddenEncoder

Hidden encoder component is used to change a value without showing it.
*/
class HiddenEncoderComponent : public Component {
protected:
    int8_t encoderId = -1;

    ValueInterface* value = NULL;

public:
    HiddenEncoderComponent(ComponentInterface::Props props)
        : Component(props)
    {
    }

    void render()
    {
    }

    bool config(char* key, char* params)
    {
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
};

#endif
