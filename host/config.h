#pragma once

#include "libs/nlohmann/json.hpp"

#include "helpers/configPlugin.h"
#include "AudioPluginHandler.h"
#include "def.h"

extern "C" void hostScriptCallback(char* key, char* value, const char* filename, std::vector<Var> variables)
{
    if (strcmp(key, "AUDIO") == 0) {
        nlohmann::json config = nlohmann::json::parse(value);
        AudioPluginHandler::get().config(config);
    } else if (strcmp(key, "AUDIO_PLUGIN") == 0) {
        AudioPluginHandler::get().loadPlugin(value);
    } else if (strcmp(key, "AUTO_SAVE") == 0) {
        uint32_t msInterval = atoi(value);
        if (msInterval > 0) {
            AudioPluginHandler::get().startAutoSave(msInterval);
        }
    }
}
