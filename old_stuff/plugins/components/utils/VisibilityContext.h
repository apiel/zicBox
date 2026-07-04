/** Description:
This specialized tool acts as a conditional visibility manager, determining whether a user interface element, widget, or data field should be displayed. It functions as a strict gatekeeper, ensuring that an item is only shown if every single defined rule is met.

The setup process involves reading a list of rules from a configuration source, such as a JSON file. Each rule specifies three things: which data point to check (identified by an index), what value to compare against, and which comparison condition to use.

The available comparison conditions are explicit:
1. Show when the value is equal to the target.
2. Show when the value is not equal to the target.
3. Show when the value is over (greater than) the target.
4. Show when the value is under (less than than) the target.

The core functionality continuously monitors incoming data updates. When new data arrives, the system runs through all stored rules. If the current data successfully satisfies all of its assigned rules simultaneously, the item remains visible. If even one condition fails, the system immediately updates the status, and the item becomes hidden.

sha: 8c45b3de2bd9b32508c56206c50b58d3e8d8ca6a6d1d4feb4409637d3d347668 
*/
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <string.h>

#include "libs/nlohmann/json.hpp"

class VisibilityContext {
public:
    enum Condition {
        SHOW_WHEN,
        SHOW_WHEN_NOT,
        SHOW_WHEN_OVER,
        SHOW_WHEN_UNDER
    } cond
        = SHOW_WHEN;

    struct ContextCondition {
        int16_t index;
        float value;
        Condition cond;
        bool isVisible = true;
    };

    std::vector<ContextCondition> contextConditions;
    bool visible = true;

    bool onContext(uint8_t index, float value)
    {
        bool update = false;
        visible = true;
        for (auto& cond : contextConditions) {
            if (index == cond.index) {
                if (cond.cond == SHOW_WHEN_NOT) {
                    cond.isVisible = value != cond.value;
                } else if (cond.cond == SHOW_WHEN_OVER) {
                    cond.isVisible = value > cond.value;
                } else if (cond.cond == SHOW_WHEN_UNDER) {
                    cond.isVisible = value < cond.value;
                } else {
                    cond.isVisible = value == cond.value;
                }
                update = true;
            }
            visible = visible && cond.isVisible;
        }
        return update;
    }

    void init(nlohmann::json& config) {
        if (config.contains("visibilityContext") && config["visibilityContext"].is_array()) {
            for (auto& context : config["visibilityContext"]) {
                ContextCondition cond;
                std::string condition = context["condition"].get<std::string>();
                if (condition == "SHOW_WHEN_NOT") {
                    cond.cond = SHOW_WHEN_NOT;
                } else if (condition == "SHOW_WHEN_OVER") {
                    cond.cond = SHOW_WHEN_OVER;
                } else if (condition == "SHOW_WHEN_UNDER") {
                    cond.cond = SHOW_WHEN_UNDER;
                } else {
                    cond.cond = SHOW_WHEN;
                }
                cond.index = context["index"].get<int16_t>();
                cond.value = context["value"].get<float>();
                contextConditions.push_back(cond);
            }
        }
    }
};
