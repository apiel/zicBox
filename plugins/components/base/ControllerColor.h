#pragma once

#include "libs/nlohmann/json.hpp"
#include "log.h"
#include "plugins/components/componentInterface.h"
#include "helpers/controller.h"

class ControllerColor {
public:
    ComponentInterface* component;

    ControllerColor(ComponentInterface* component)
        : component(component)
    {
    }

    void init(nlohmann::json& config)
    {
        if (config.contains("controllerColors") && config["controllerColors"].is_array()) {
            for (auto& controllerColor : config["controllerColors"]) {
                if (!controllerColor.contains("controller")) {
                    logWarn("Controller color config is missing mandatory 'controller' (id) parameter.");
                    continue;
                }
                if (!controllerColor.contains("colors")) {
                    logWarn("Controller color config is missing mandatory 'colors' parameter.");
                    continue;
                }
                std::string controllerId = controllerColor["controller"].get<std::string>();
                nlohmann::json& colors = controllerColor["colors"];
                addColors(controllerId, colors);
            }
        }
    }

    struct ControllerColors {
        ControllerInterface* controller;
        int key;
        Color color;
    };
    std::vector<ControllerColors> controllerColors;

    void addColors(std::string controllerId, nlohmann::json& colors)
    {
        if (!colors.is_array()) {
            logWarn("Controller color 'colors' parameter must be an array.");
            return;
        }

        ControllerInterface* controller = component->getController(controllerId.c_str());
        if (!controller) {
            logWarn("Controller color config is missing mandatory 'controller' (id) parameter.");
            return;
        }

        for (auto& color : colors) {
            if (!color.contains("key") || !color.contains("color")) {
                logWarn("Controller color config is missing mandatory 'key' or 'color' parameters.");
                continue;
            }
            int key = color["key"].is_number_integer() ? color["key"].get<int>() : getKeyCode(color["key"].get<std::string>().c_str());
            Color colorValue = component->draw.getColor(color["color"], { 0, 0, 0 });
            controllerColors.push_back({ controller, key, colorValue });
        }
    }

    void render()
    {
        logInfo("Call render Controller colors: %d", controllerColors.size());
        for (auto& controllerColor : controllerColors) {
            logInfo("Controller color: %d r %d g %d b %d", controllerColor.key, controllerColor.color.r, controllerColor.color.g, controllerColor.color.b);
            controllerColor.controller->setColor(controllerColor.key, controllerColor.color);
        }
    }
};
