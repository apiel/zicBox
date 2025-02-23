#pragma once

#include "controllers.h"
#ifdef _UI_SDL_EVENT_HANDLER_H_
#include "SDL_EventHandler.h"
#endif
#include "host.h"
#include "log.h"
#include "plugins/components/drawInterface.h"
#include "styles.h"
#include "viewManager.h"

#include "plugins/components/utils/color.h"

#include <fstream>

void loadJsonConfig(std::string configPath)
{
    try {
        std::ifstream configFile(configPath);
        if (configFile.is_open()) {
            nlohmann::json config = nlohmann::json::parse(configFile);
            configFile.close();
            if (config.contains("audio")) {
                hostConfig(config["audio"]);
            }
            lastPluginControllerInstance->config(config); // <--- not very nice!!!!
            if (config.contains("controllers")) {
                // TODO to be implemented...
            }
            // Should happen before views
            if (config.contains("screen")) {
                ViewManager::get().draw.config(config["screen"]);
            }
            if (config.contains("views")) {
                ViewManager::get().config(config["views"]);
            }
        }
    } catch (const std::exception& e) {
        logError("load json config: %s", e.what());
    }
}