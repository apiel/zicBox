#ifndef _UI_CONFIG_H_
#define _UI_CONFIG_H_

#include "controllers.h"
#include "host.h"
#include "styles.h"
#include "viewManager.h"

#include "dustscript/dustscript.h"

void uiScriptCallback(char* key, char* value, const char* filename, uint8_t indentation, DustScript& instance)
{
    if (strcmp(key, "print") == 0) {
        printf(">> LOG: %s\n", value);
    } else if (strcmp(key, "INCLUDE") == 0) {
        char fullpath[512];
        getFullpath(value, filename, fullpath);
        DustScript::load(fullpath, uiScriptCallback);
    } else if (strcmp(key, "LOAD_HOST") == 0) {
        char hostConfigPath[512];
        getFullpath(value, filename, hostConfigPath);
        if (!loadHost(hostConfigPath)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load host");
        }
    } else if (strcmp(key, "SCREEN") == 0) {
        styles.screen.w = atoi(strtok(value, " "));
        styles.screen.h = atoi(strtok(NULL, " "));
    } else if (strcmp(key, "PLUGIN_CONTROLLER") == 0) {
        loadPluginController(value, filename);
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

void loadUiConfig(const char* filename)
{
    DustScript::load(filename, uiScriptCallback, { .variables = { { "$IS_RPI",
#ifdef IS_RPI
                                                       "true"
#else
                                                       "false"
#endif
                                                   } } });
}

#endif