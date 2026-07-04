/** Description:
This code defines a specialized system responsible for managing and applying colors to various interactive elements, often referred to as "controllers" (like physical buttons or visual interface parts).

The main component is a manager that acts as a bridge between user configuration and hardware output.

**How It Works:**

1.  **Setup and Configuration:** When the system starts, the manager reads a setup file (a configuration list). It specifically looks for instructions defining which controllers should display custom colors.
2.  **Color Assignment:** For each listed controller, it identifies the specific key or element that needs coloring. It then translates the user-defined color name (e.g., "red") into the precise digital color value the device understands. This system can handle key identifiers given as characters, names, or numbers.
3.  **Data Storage:** All these color-to-key-to-controller mappings are stored internally in a structured list.
4.  **Display (Rendering):** During operation, the system uses a dedicated function to cycle through this stored list. For every entry, it sends a direct command to the corresponding controller, instructing it to immediately set that specific key or element to the designated color.

Essentially, this module automates the entire process of reading customized color schemes and actively displaying them on hardware or interface elements without needing manual intervention.

sha: 59cd4289695223fca72b9a391947ed885e6f3cd9b2ea1c7a6a1454e1e694690c 
*/
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
