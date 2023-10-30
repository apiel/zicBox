#ifndef _UI_CONFIG_H_
#define _UI_CONFIG_H_

#include "controllers.h"
#include "helpers/config.h"
#include "viewManager.h"

void uiConfigKeyValue(char* key, char* value, const char* filename)
{
    if (strcmp(key, "INCLUDE") == 0) {
        char fullpath[512];
        getFullpath(value, filename, fullpath);
        loadConfig(fullpath, uiConfigKeyValue);
    } else if (strcmp(key, "PLUGIN_CONTROLLER") == 0) {
        char fullpath[512];
        getFullpath(value, filename, fullpath);
        loadPluginController(fullpath);
    } else if (strcmp(key, "SET_COLOR") == 0) {
        char* name = strtok(value, " ");
        char* color = strtok(NULL, " ");
        ViewManager::get().draw.setColor(name, color);
    } else if (lastPluginInstance && lastPluginInstance->config(key, value)) {
        return;
    } else {
        ViewManager::get().config(key, value, filename);
    }
}

bool loadUiConfig()
{
    return loadConfig("./ui/index.ui", uiConfigKeyValue);
}

#endif