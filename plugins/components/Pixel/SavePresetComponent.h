#pragma once

#include "plugins/components/base/Input.h"

#include <fstream>

class SavePresetComponent : public Input {
protected:
    AudioPlugin* audioPlugin = nullptr;
    std::string folder = "data/presets";
    std::string redirectView = "&previous";

public:
    SavePresetComponent(ComponentInterface::Props props)
        : Input(props)
    {
        nlohmann::json& config = props.config;
        audioPlugin = getPluginPtr(config, "audioPlugin", track); //eg: "audio_plugin_name"

        folder = config.value("folder", folder);
        redirectView = config.value("redirectView", redirectView);
    }

    void save(std::string inputValue) override
    {
        // logDebug("save preset: %s", inputValue.c_str());
        if (!audioPlugin) {
            return;
        }

        if (inputValue.empty()) {
            // generate random value
            value = "preset_";
            int range = charset.size() - charsetSpecial.size();
            for (int i = 0; i < 6; i++) {
                value += charset[rand() % range];
            }
            cursorPos = value.size();
            renderNext();
            return;
        }

        nlohmann::json json;
        audioPlugin->serializeJson(json);
        // logDebug("json: %s", json.dump(4).c_str());

        std::filesystem::create_directories(folder);
        std::string filepath = folder + "/" + inputValue + ".json";
        std::ofstream file(filepath);
        file << json.dump(4);
        file.close();

        if (redirectView != "") {
            view->setView(redirectView);
        }
    }
};
