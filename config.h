#ifndef _UI_CONFIG_H_
#define _UI_CONFIG_H_

#include "controllers.h"
#include "viewManager.h"

#include "dustscript/dustscript.h"

void scriptCallback(char* key, char* value, const char* filename, uint8_t indentation)
{
    if (strcmp(key, "print") == 0) {
        printf(">> LOG: %s\n", value);
    } else if (strcmp(key, "INCLUDE") == 0) {
        char fullpath[512];
        getFullpath(value, filename, fullpath);
        DustScript::load(fullpath, scriptCallback);
    } else if (strcmp(key, "PLUGIN_CONTROLLER") == 0) {
        char fullpath[512];
        getFullpath(value, filename, fullpath);
        loadPluginController(fullpath);
    } else if (strcmp(key, "SET_COLOR") == 0) {
        char* name = strtok(value, " ");
        char* color = strtok(NULL, " ");
        ViewManager::get().draw.setColor(name, color);
    } else if (pluginControllerConfig(key, value)) {
        return;
    } else {
        ViewManager::get().config(key, value, filename);
    }
}

void loadUiConfig()
{
    DustScript::load("ui/index.ui", scriptCallback);
}

#endif