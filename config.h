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
        logInfo("load json config: %s", configPath.c_str());
        if (configFile.is_open()) {
            nlohmann::json config = nlohmann::json::parse(configFile);
            configFile.close();
            if (config.contains("audio")) {
                logInfo("----------- init audio -------------");
                hostConfig(config["audio"]);
            }
            logInfo("----------- init controllers -------------");
            lastPluginControllerInstance->config(config); // <--- not very nice!!!!
            if (config.contains("controllers")) {

                // TODO to be implemented...
            }

            ViewManager::get().config(config);
        }
        logInfo("----------- config done -------------");
    } catch (const std::exception& e) {
        logError("load json config: %s", e.what());
    }
}