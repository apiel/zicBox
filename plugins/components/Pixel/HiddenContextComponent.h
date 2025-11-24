/** Description:
This file defines a specialized control mechanism called the `HiddenContextComponent`. Its primary function is to modify a specific setting or variable within a larger system—known as a "context"—without needing a visual user interface element. It operates entirely in the background.

**How It Works:**

This component acts as an intermediary, translating physical input into system adjustments.

1.  **Setup and Configuration:** When initialized, it reads its settings from a configuration file. These settings include which physical input device (like a rotary knob or encoder) it should listen to (`encoderId`), the specific system setting it controls (`contextId`), and the boundaries of that setting (`min` and `max` values). It also defines how much the value should change with each turn (`stepIncrementation`).
2.  **Listening for Input:** The component constantly monitors the system for movement from its assigned physical input device.
3.  **Processing:** When the physical control is turned, the component intercepts the signal. It calculates the new value by adding the defined step size. If configured, it reverses the input direction.
4.  **Safety Check:** A critical function ensures the new value never exceeds the pre-set minimum and maximum limits.
5.  **Updating the System:** Finally, it updates the designated system setting (the "context variable") with the newly calculated, safe value.

In summary, this component provides a dedicated, non-visual way to map a physical control directly to a bounded system parameter, ensuring robust and accurate control over hidden settings.

sha: cca07a282ee29c4450abb44b8d85612384cb1a78cf4593915d2dac8759e8dfa1 
*/
#pragma once

#include "plugins/components/component.h"
#include "helpers/clamp.h"

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

    void onEncoder(int8_t id, int8_t direction) override
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
