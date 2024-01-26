#ifndef _UI_COMPONENT_BUTTON_H_
#define _UI_COMPONENT_BUTTON_H_

#include "base/ButtonBaseComponent.h"

/*md
## Button

Push button component that trigger an action when pressed and released.
*/
class ButtonComponent : public ButtonBaseComponent {
protected:
    void set(std::function<void()>& event, char* config)
    {
        char* action = strtok(config, " ");

        if (strcmp(action, "&SET_VIEW") == 0) {
            // char* name = strtok(NULL, " ");
            // // char* nameCopy = new char[strlen(name) + 1];
            // // strcpy(nameCopy, name);
            // std::string nameCopy(name);
            // event = [this, nameCopy]() {
            //     this->setView(nameCopy);
            // };
            std::string name = strtok(NULL, " ");
            event = [this, name]() {
                this->setView(name);
            };
            return;
        }

        if (strcmp(action, "&NOTE_ON") == 0) {
            char* pluginName = strtok(NULL, " ");
            char* noteStr = strtok(NULL, " ");
            uint8_t note = atoi(noteStr);
            char* velocityStr = strtok(NULL, " ");
            float velocity = atof(velocityStr);
            AudioPlugin* plugin = &getPlugin(pluginName, track);
            event = [plugin, note, velocity]() {
                plugin->noteOn(note, velocity);
            };
            return;
        }

        if (strcmp(action, "&NOTE_OFF") == 0) {
            char* pluginName = strtok(NULL, " ");
            char* noteStr = strtok(NULL, " ");
            uint8_t note = atoi(noteStr);
            AudioPlugin* plugin = &getPlugin(pluginName, track);
            event = [plugin, note]() {
                plugin->noteOff(note, 0);
            };
            return;
        }

        if (strcmp(action, "&DATA") == 0) {
            char* pluginName = strtok(NULL, " ");
            int dataId = atoi(strtok(NULL, " "));
            AudioPlugin* plugin = &getPlugin(pluginName, track);
            event = [plugin, dataId]() {
                plugin->data(dataId);
            };
            return;
        }

        char* key = strtok(NULL, " ");
        char* targetValue = strtok(NULL, " ");

        ValueInterface* value = val(getPlugin(action, track).getValue(key));
        if (value != NULL && label == "") {
            label = value->label();
        } else {
            // printf("No value %s %s found for button on track %d\n", action, key, track);
        }

        float target = atof(targetValue);
        event = [value, target]() {
            value->set(target);
        };
    }

public:
    ButtonComponent(ComponentInterface::Props props)
        : ButtonBaseComponent(props)
    {
    }

    bool config(char* key, char* value)
    {
/*md
- `ON_PRESS: action` is used to set an action when the button is pressed
    - `ON_PRESS: &SET_VIEW name` sets the specified view when the button is pressed
    - `ON_PRESS: &NOTE_ON pluginName note velocity` send the note on the specified plugin when the button is pressed
    - `ON_PRESS: &NOTE_OFF pluginName note` send the note off the specified plugin when the button is pressed
    - `ON_PRESS: &DATA pluginName dataId` send the data to the specified plugin when the button is pressed
    - `ON_PRESS: pluginName key value` sets the value of the specified plugin key when the button is pressed
*/
        if (strcmp(key, "ON_PRESS") == 0) {
            // printf("value: %s\n", value);
            set(onPress, value);
            return true;
        }

        /*md - `ON_RELEASE: action` is used to set an action when the button is released  (where action can be the same as ON_PRESS)*/
        if (strcmp(key, "ON_RELEASE") == 0) {
            // printf("value: %s\n", value);
            set(onRelease, value);
            return true;
        }

        return ButtonBaseComponent::config(key, value);
    }
};

#endif
