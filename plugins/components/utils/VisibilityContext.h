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
