#pragma once

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

public:
    HiddenValueComponent(ComponentInterface::Props props)
        : Component(props)
    {
        /*md md_config:HiddenValue */
        nlohmann::json& config = props.config;

        /// The audio plugin to get control on.
        if (config.contains("audioPlugin")) {
            std::string audioPlugin = config["audioPlugin"].get<std::string>(); //eg: "audio_plugin_name"
            /// The audio plugin key parameter to get control on.
            if (config.contains("param")) {
                std::string param = config["param"].get<std::string>(); //eg: "parameter_name"
                value = getPluginPtr(config, "audioPlugin", track)->getValue(param);
            }
        }

        /// The encoder id that will interract with this component.
        encoderId = config.value("encoderId", encoderId); //eg: 0

        /// Invert the encoder direction
        inverted = config.value("inverted", inverted); //eg: true

        /*md md_config_end */
    }

    void render()
    {
    }

    void onEncoder(int id, int8_t direction) override
    {
        if (id == encoderId) {
            if (inverted) {
                direction = -direction;
            }
            if (value) {
                value->increment(direction);
            }
        }
    }
};
