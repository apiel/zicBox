#pragma once

#include "plugins/components/base/Input.h"

class SavePresetComponent : public Input {
protected:

public:
    SavePresetComponent(ComponentInterface::Props props)
        : Input(props)
    {
    }

    void save(std::string inputValue) override
    {
        logDebug("save preset: %s", inputValue.c_str());
    }
};
