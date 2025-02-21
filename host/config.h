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
    }
}
