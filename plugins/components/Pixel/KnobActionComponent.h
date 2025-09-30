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

    void onEncoder(int id, int8_t direction)
    {
        if (isVisible() && id == encoderId && fakeKeymap.action != nullptr) {
            fakeKeymap.action(fakeKeymap);
            if (fakeKeymap.action2 != nullptr) {
                fakeKeymap.action2(fakeKeymap);
            }
        }
    }
};
