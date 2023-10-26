#ifndef _UI_CONFIG_H_
#define _UI_CONFIG_H_

#include "helpers/config.h"
#include "viewManager.h"
#include "controllers.h"

void uiConfigKeyValue(char *key, char *value, const char *filename)
{
    if (strcmp(key, "INCLUDE") == 0)
    {
        loadConfig(getFullpath(value, filename), uiConfigKeyValue);
    }
    else if (strcmp(key, "PLUGIN_CONTROLLER") == 0)
    {
        loadPluginController(getFullpath(value, filename));
    }
    else if (strcmp(key, "SET_COLOR") == 0)
    {
        char *name = strtok(value, " ");
        char *color = strtok(NULL, " ");
        ViewManager::get().draw.setColor(name, color);
    }
    else if (lastPluginInstance && lastPluginInstance->config(key, value))
    {
        return;
    }
    else
    {
        ViewManager::get().config(key, value, filename);
    }
}

bool loadUiConfig()
{
    return loadConfig("./ui/index.ui", uiConfigKeyValue);
}

#endif