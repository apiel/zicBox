#ifndef _UI_COMPONENT_BUTTON_H_
#define _UI_COMPONENT_BUTTON_H_

#include "base/ButtonBaseComponent.h"

class ButtonComponent : public ButtonBaseComponent {
protected:
    void set(std::function<void()>& event, char* config)
    {
        char* action = strtok(config, " ");

        if (strcmp(action, "&SET_VIEW") == 0) {
            char* name = strtok(NULL, " ");
            char* nameCopy = new char[strlen(name) + 1];
            strcpy(nameCopy, name);
            event = [this, nameCopy]() {
                this->setView(nameCopy);
            };
            return;
        }

        if (strcmp(action, "&NOTE_ON") == 0) {
            char* pluginName = strtok(NULL, " ");
            AudioPlugin* plugin = &getPlugin(pluginName);
            char* noteStr = strtok(NULL, " ");
            uint8_t note = atoi(noteStr);
            char* velocityStr = strtok(NULL, " ");
            uint8_t velocity = atoi(velocityStr);
            event = [plugin, note, velocity]() {
                plugin->noteOn(note, velocity);
            };
            return;
        }

        if (strcmp(action, "&NOTE_OFF") == 0) {
            char* pluginName = strtok(NULL, " ");
            AudioPlugin* plugin = &getPlugin(pluginName);
            char* noteStr = strtok(NULL, " ");
            uint8_t note = atoi(noteStr);
            event = [plugin, note]() {
                plugin->noteOff(note, 0);
            };
            return;
        }

        char* key = strtok(NULL, " ");
        char* targetValue = strtok(NULL, " ");

        ValueInterface* value = val(getPlugin(action).getValue(key));
        if (value != NULL && label == "") {
            label = value->label();
        }

        float target = atof(targetValue);
        event = [value, target]() {
            value->set(target);
        };
    }

public:
    ButtonComponent(ComponentInterface::Props& props)
        : ButtonBaseComponent(props)
    {
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "ON_PRESS") == 0) {
            // printf("value: %s\n", value);
            set(onPress, value);
            return true;
        }

        if (strcmp(key, "ON_RELEASE") == 0) {
            // printf("value: %s\n", value);
            set(onRelease, value);
            return true;
        }

        return ButtonBaseComponent::config(key, value);
    }
};

#endif
