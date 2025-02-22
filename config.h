#ifndef _UI_CONFIG_H_
#define _UI_CONFIG_H_

#include "controllers.h"
#ifdef _UI_SDL_EVENT_HANDLER_H_
#include "SDL_EventHandler.h"
#endif
#include "host.h"
#include "log.h"
#include "plugins/components/drawInterface.h"
#include "styles.h"
#include "viewManager.h"

#include "helpers/configPlugin.h"
#include "helpers/getFullpath.h"
#include "plugins/components/utils/color.h"
#include "plugins/config/LuaConfig.h"

#include <fstream>
std::ofstream logFile;
bool logConfigToFile = true;
void logScript(char* key, char* value)
{
    if (logConfigToFile) {
        if (!logFile.is_open()) {
            logFile.open("pixel.log", std::ios::trunc);
        }
        if (logFile.is_open()) {
            logFile << (key ? key : "null") << ": " << (value ? value : "null") << std::endl;
        }
    }
}
void closeLog()
{
    if (logFile.is_open()) {
        logFile.close();
    }
}

void uiScriptCallback(char* key, char* value, const char* filename, std::vector<Var> variables)
{
    logScript(key, value);
    if (strcmp(key, "print") == 0 || strcmp(key, "PRINT") == 0) {
        printf(">> LOG: %s\n", value);
    } else if (strcmp(key, "LOG_CONFIG") == 0) {
        logConfigToFile = (strcmp(value, "true") == 0);
    } else if (strcmp(key, "LOAD_CONFIG") == 0) {
        char* script = strtok(value, " ");
        char* plugin = strtok(NULL, " ");
        loadConfigPlugin(getFullpath(plugin, filename).c_str(), script, uiScriptCallback, variables);
    } else if (strcmp(key, "PLUGIN_CONTROLLER") == 0) {
        loadPluginController(value, filename);
    } else if (
        pluginControllerConfig(key, value)
#ifdef _UI_SDL_EVENT_HANDLER_H_
        || EventHandler::get().config(key, value)
#endif
        || ViewManager::get().config(key, value, filename)) {
        return;
    } else if (strcmp(key, "AUDIO") == 0) {
        nlohmann::json config = nlohmann::json::parse(value);
        hostConfig(config);
    }
}

void loadUiConfig(const char* scriptPath, const char* pluginPath, Styles styles)
{
    std::vector<Var> variables = {
        { "SCREEN_WIDTH", std::to_string(styles.screen.w) },
        { "SCREEN_HEIGHT", std::to_string(styles.screen.h) },
        { "PRIMARY_COLOR", rgbToString(styles.colors.primary) },
        { "SECONDARY_COLOR", rgbToString(styles.colors.secondary) },
        { "TERTIARY_COLOR", rgbToString(styles.colors.tertiary) },
        { "TEXT_COLOR", rgbToString(styles.colors.text) },
        { "BACKGROUND_COLOR", rgbToString(styles.colors.background) },
    };
    loadConfigPlugin(pluginPath, scriptPath, uiScriptCallback, variables);
    closeLog();
}

#endif