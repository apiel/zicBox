#ifndef _UI_CONFIG_H_
#define _UI_CONFIG_H_

#include "controllers.h"
#include "viewManager.h"

#include "dustscript/dustscript.h"

void scriptCallback(char* command, std::vector<string> params, const char* filename, uint8_t indentation)
{
    if (strcmp(command, "print") == 0) {
        printf(">> LOG: %s\n", params[0].c_str());
    } else if (strcmp(command, "INCLUDE") == 0) {
        char fullpath[512];
        getFullpath((char*)params[0].c_str(), filename, fullpath);
        DustScript::load(fullpath, scriptCallback);
    } else if (strcmp(command, "PLUGIN_CONTROLLER") == 0) {
        char fullpath[512];
        getFullpath((char*)params[0].c_str(), filename, fullpath);
        loadPluginController(fullpath);
    } else if (strcmp(command, "SET_COLOR") == 0) {
        ViewManager::get().draw.setColor((char*)params[0].c_str(), (char*)params[1].c_str());
    } else if (pluginControllerConfig(command, params)) {
        return;
    } else {
        ViewManager::get().config(command, (char*)params[0].c_str(), filename); // FIXME
    }
}

// void uiConfigKeyValue(char* key, char* value, const char* filename)
// {
//     if (strcmp(key, "INCLUDE") == 0) {
//         char fullpath[512];
//         getFullpath(value, filename, fullpath);
//         loadConfig(fullpath, uiConfigKeyValue);
//     } else if (strcmp(key, "PLUGIN_CONTROLLER") == 0) {
//         char fullpath[512];
//         getFullpath(value, filename, fullpath);
//         loadPluginController(fullpath);
//     } else if (strcmp(key, "SET_COLOR") == 0) {
//         char* name = strtok(value, " ");
//         char* color = strtok(NULL, " ");
//         ViewManager::get().draw.setColor(name, color);
//     } else if (lastPluginInstance && lastPluginInstance->config(key, value)) {
//         return;
//     } else {
//         ViewManager::get().config(key, value, filename);
//     }
// }

void loadUiConfig()
{
    DustScript::load("ui/index.dust", scriptCallback);
}

#endif