/** Description:
This specialized component, named `SavePresetComponent`, is designed to handle the critical function of saving user-defined settings within an application, likely an audio software or plugin environment. It acts as a customized input field or button that triggers a permanent storage operation.

### Core Functionality

This component serves as a bridge between the user interface and the underlying processing engine (the "Audio Plugin").

1.  **State Capture:** When the user initiates a save operation, the component immediately connects to its linked Audio Plugin. It requests that the plugin serialize, or package up, all its current parameters—like knob positions, sliders, and internal configurations—into a structured digital format (JSON).
2.  **File Management:** It uses a default or user-defined location (e.g., a "presets" folder) to store this data. The component automatically ensures that this folder exists on the computer’s file system.
3.  **Saving Data:** The packaged settings are written into a new file, using the name provided by the user (the "input value"). This allows the user to recall the exact state of the plugin later.
4.  **Error Handling and Interaction:** If the user attempts to save without providing a name, the component generates a temporary, random name suggestion to prevent accidental empty saves.
5.  **View Redirection:** Upon successful saving, the component can optionally redirect the user’s view within the application, seamlessly moving them to a different screen or menu.

In summary, this code defines a robust control responsible for capturing, naming, organizing, and writing the complex operational settings of an audio component to a safe location on disk.

sha: 6710be8248bd00b457fdea354b944ce45072b4967c347f2ef7a8d1b95a76f1df 
*/
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
