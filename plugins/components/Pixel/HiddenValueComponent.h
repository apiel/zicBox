/** Description:
This C++ header defines a foundational element for a plugin system known as the "Hidden Value Component." Its purpose is to allow developers to modify internal settings or specific parameters within audio software or hardware without requiring a visible user interface element (like a button, slider, or display).

**Core Functionality**

The component acts as an invisible bridge between a physical input, typically a rotary encoder (a type of digital knob), and a configurable value in the system. Because its `render` function is empty, it has no visual display associated with it.

**Configuration and Setup**

When this component is initialized, it is configured using properties:

1.  **Input Linkage:** It must be assigned an `encoderId`, which identifies which physical knob or dial on the hardware interface controls this component.
2.  **Value Target:** Optionally, it can be linked to a specific parameter within an external audio plugin (for example, controlling the volume or filter cutoff of a synthesizer). If no specific plugin parameter is defined, it controls a default internal group setting.
3.  **Direction Control:** A setting allows the user to `invert` the encoder's direction. For instance, turning the knob clockwise might normally increase the value, but with inversion, it would decrease it.

**Operation**

When the user turns the linked knob, the component immediately registers the movement. It verifies the direction, applies any configured inversion, and then sends a command directly to the target value (the plugin parameter or internal setting) to increment or decrement itself accordingly. This provides immediate, silent control over backend variables.

sha: b269efb050a74e04b60826dab2574e0901baa772acbb7cffbef8b36f38d79716 
*/
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

    void onEncoder(int8_t id, int8_t direction) override
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

    const std::vector<EventInterface::EncoderPosition> getEncoderPositions() override
    {
        if (encoderId < 0) {
            return {};
        }
        
        return {
            { encoderId, size, relativePosition },
        };
    }
};
