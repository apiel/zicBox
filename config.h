#ifndef _UI_CONFIG_H_
#define _UI_CONFIG_H_

#include "controllers.h"
#ifdef _UI_SDL_EVENT_HANDLER_H_
#include "SDL_EventHandler.h"
#endif
#include "host.h"
#include "log.h"
#include "styles.h"
#include "viewManager.h"

#include "helpers/configPlugin.h"

void uiScriptCallback(char* key, char* value, const char* filename)
{
    if (strcmp(key, "print") == 0) {
        printf(">> LOG: %s\n", value);
    } else if (strcmp(key, "LOAD_HOST") == 0) {
        std::string configPath = strtok(value, " ");
        char* pluginConfig = strtok(NULL, " ");
        if (!loadHost(getFullpath(value, filename), pluginConfig)) {
            logError("Could not load host");
        }
    } else if (strcmp(key, "SCREEN") == 0) {
        styles.screen.w = atoi(strtok(value, " "));
        styles.screen.h = atoi(strtok(NULL, " "));
    } else if (strcmp(key, "PLUGIN_CONTROLLER") == 0) {
        loadPluginController(value, filename);
    } else if (strcmp(key, "LOAD_CONFIG_PLUGIN") == 0) {
        char* scriptPath = strtok(value, " ");
        char* plugin = strtok(NULL, " ");
        loadConfigPlugin(scriptPath, plugin, uiScriptCallback);
    } else if (
        pluginControllerConfig(key, value)
#ifdef _UI_SDL_EVENT_HANDLER_H_
        || EventHandler::get().config(key, value)
#endif
        || ViewManager::get().draw.config(key, value)) {
        return;
    } else {
        ViewManager::get().config(key, value, filename);
    }
}

void loadUiConfig(const char* scriptPath, const char* pluginPath)
{
    loadConfigPlugin(pluginPath, scriptPath, uiScriptCallback);
}

#endif