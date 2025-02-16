#ifndef _UI_COMPONENT_HIDDEN_VALUE_H_
#define _UI_COMPONENT_HIDDEN_VALUE_H_

#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"

#include <string>

/*md
## HiddenValue

Hidden value component is used to change a value without to have a specific UI for it.
By default it will update the group number. If a plugin parameter is set, it will update the value of this parameter.
*/
class HiddenValueComponent : public Component {
protected:
    int8_t encoderId = -1;

    ValueInterface* value = NULL;
    bool inverted = false;

    KeypadLayout keypadLayout;

public:
    HiddenValueComponent(ComponentInterface::Props props)
        : Component(props)
        , keypadLayout(this)
    {
        /*md md_config:HiddenValue */
        nlohmann::json config = props.config;

        /*md   // The audio plugin to get control on. */
        /*md   audioPlugin="audio_plugin_name" */
        if (config.contains("audioPlugin")) {
            std::string audioPlugin = config["audioPlugin"].get<std::string>();
            /*md   // The audio plugin key parameter to get control on. */
            /*md   param="parameter_name" */
            if (config.contains("param")) {
                std::string param = config["param"].get<std::string>();
                value = getPlugin(audioPlugin.c_str(), track).getValue(param);
            }
        }

        /*md   // The encoder id that will interract with this component. */
        /*md   encoderId={0} */
        encoderId = config.value("encoderId", encoderId);

        /*md  // Invert the encoder direction */
        /*md  inverted */
        inverted = config.value("inverted", inverted);

        /*md md_config_end */
    }

    void render()
    {
    }

    int8_t currentGroup = 0;
    void onGroupChanged(int8_t index) override
    {
        currentGroup = index;
    }

    void onEncoder(int id, int8_t direction) override
    {
        if (id == encoderId) {
            if (inverted) {
                direction = -direction;
            }
            if (value) {
                value->increment(direction);
            } else { // By default it will change group
                view->setGroup(currentGroup + (direction > 0 ? 1 : -1));
            }
        }
    }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        keypadLayout.onKey(id, key, state, now);
    }

    bool config(char* key, char* value) override
    {
        if (keypadLayout.config(key, value)) {
            return true;
        }

        return false;
    }
};

#endif
