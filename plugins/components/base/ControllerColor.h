#pragma once

#include "helpers/controller.h"
#include "libs/nlohmann/json.hpp"
#include "log.h"
#include "plugins/components/componentInterface.h"

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
    std::vector<ControllerInterface*> controllers;

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

        controllers.push_back(controller);
        for (auto& color : colors) {
            if (!color.contains("key") || !color.contains("color")) {
                logWarn("Controller color config is missing mandatory 'key' or 'color' parameters.");
                continue;
            }
            int key = -1;
            if (color["key"].is_number_integer()) {
                key = color["key"].get<uint8_t>();
            } else {
                std::string keyStr = color["key"].get<std::string>();
                if (keyStr.length() == 1) {
                    keyStr = "'" + keyStr + "'";
                }
                key = getKeyCode(keyStr.c_str());
            }

            Color colorValue = component->draw.getColor(color["color"], { 0, 0, 0 });
            controllerColors.push_back({ controller, key, colorValue });
        }
    }

    void render()
    {
        for (auto& controllerColor : controllerColors) {
            controllerColor.controller->setColor(controllerColor.key, controllerColor.color);
        }
    }
};
