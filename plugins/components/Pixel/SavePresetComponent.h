#pragma once

#include "plugins/components/base/Input.h"

class SavePresetComponent : public Input {
protected:
    AudioPlugin* audioPlugin = nullptr;

public:
    SavePresetComponent(ComponentInterface::Props props)
        : Input(props)
    {
        nlohmann::json& config = props.config;
        audioPlugin = getPluginPtr(config, "audioPlugin", track); //eg: "audio_plugin_name"
    }

    void save(std::string inputValue) override
    {
        logDebug("save preset: %s", inputValue.c_str());
        if (!audioPlugin) {
            return;
        }
        nlohmann::json json;
        audioPlugin->serializeJson(json);
        logDebug("json: %s", json.dump(4).c_str());
    }
};
