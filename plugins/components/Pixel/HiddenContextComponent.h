#pragma once

#include "plugins/components/component.h"
#include "helpers/range.h"

#include <string>

/*md
## HiddenContext

Hidden context component is used to change a context without to have a specific UI for it.
*/
class HiddenContextComponent : public Component {
protected:
    int8_t encoderId = -1;
    float min = 0;
    float max = 100;
    uint8_t contextId = 0;
    bool inverted = false;
    float stepIncrementation = 1.0f;

public:
    HiddenContextComponent(ComponentInterface::Props props)
        : Component(props)
    {
        /*md md_config:HiddenValue */
        nlohmann::json& config = props.config;

        /// The encoder id that will interract with this component.
        encoderId = config.value("encoderId", encoderId); //eg: 0

        /// Invert the encoder direction
        inverted = config.value("inverted", inverted); //eg: true

        /// Set min value
        min = config.value("min", min); //eg: 0

        /// Set max value
        max = config.value("max", max); //eg: 100

        /// Set step incrementation
        stepIncrementation = config.value("stepIncrementation", stepIncrementation); //eg: 1.0

        /// Set context id
        contextId = config.value("contextId", contextId); //eg: 0

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
            // printf("[HiddenContextComponent] before %f\n", view->contextVar[contextId]);
            setContext(contextId, CLAMP(view->contextVar[contextId] + direction * stepIncrementation, min, max));
            // printf("[HiddenContextComponent] after %f\n", view->contextVar[contextId]);
        }
    }
};
