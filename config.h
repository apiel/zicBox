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
#include "plugins/components/utils/color.h"

void uiScriptCallback(char* key, char* value, const char* filename, std::vector<Var> variables)
{
    if (strcmp(key, "print") == 0) {
        printf(">> LOG: %s\n", value);
    } else if (strcmp(key, "PLUGIN_CONTROLLER") == 0) {
        loadPluginController(value, filename);
    } else if (
        pluginControllerConfig(key, value)
#ifdef _UI_SDL_EVENT_HANDLER_H_
        || EventHandler::get().config(key, value)
#endif
        || ViewManager::get().config(key, value, filename)) {
        return;
    } else {
        hostScriptCallback(key, value, filename, variables);
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
}

#endif