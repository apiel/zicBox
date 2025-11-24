/** Description:
This file defines a specialized control module called `KnobActionComponent`. This component is designed to integrate physical rotary input devices, often referred to as encoders or knobs, into a larger software system.

The core function of this component is to translate the turning motion of a specific hardware knob into one or more predefined software commands or "actions."

**How It Works:**

1.  **Initialization:** When the system starts, the component reads its configuration. It determines the unique identification number (`encoderId`) of the rotary knob it needs to monitor. It also identifies the names of the commands (`action` and an optional second command `action2`) that should be executed.

2.  **Action Mapping:** To efficiently execute these commands, the component uses an internal structure that mimics a standard keyboard input or key press. This allows it to reuse existing system functions designed for button presses, effectively simulating a virtual key activation when the physical knob is turned.

3.  **Execution:** The module continuously monitors the specified knob. If the knob is rotated and the component is currently active, it instantly triggers the assigned primary action. If a secondary action was also configured, that is executed immediately afterward. This setup efficiently links physical user input directly to functional software operations.

sha: a30d0f5e3ecd7a60e7b78c99141f68439941d748a939f6162de0fba5221a3779 
*/
#pragma once

#include "log.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <math.h>
#include <string>

class KnobActionComponent : public Component {
protected:
    int8_t encoderId = -1;

    KeypadLayout::KeyMap fakeKeymap = {
        .controller = nullptr,
        .controllerId = 0,
        .key = 0,
        .action = nullptr,
        .action2 = nullptr,
        // .actionLongPress = nullptr,
        .useContext = false,
        .contextId = 0,
        .contextValue = 0.0f,
        .multipleKeyHandler = false,
        .getColor = nullptr,
        .isLongPress = false,
        .pressedTime = 0
    };

public:
    KnobActionComponent(ComponentInterface::Props props)
        : Component(props)
    {
        nlohmann::json& config = props.config;

        encoderId = config.value("encoderId", encoderId);

        if (config.contains("action")) {
            std::string actionStr = config["action"].get<std::string>();
            fakeKeymap.action = keypadLayout.getAction(actionStr);
        } else {
            logWarn("KnobActionComponent is missing mandatory parameter");
        }

        if (config.contains("action2")) {
            std::string action2Str = config["action2"].get<std::string>();
            fakeKeymap.action2 = keypadLayout.getAction(action2Str);
        }
    }

    void onEncoder(int8_t id, int8_t direction)
    {
        if (isVisible() && id == encoderId && fakeKeymap.action != nullptr) {
            fakeKeymap.action(fakeKeymap);
            if (fakeKeymap.action2 != nullptr) {
                fakeKeymap.action2(fakeKeymap);
            }
        }
    }
};
