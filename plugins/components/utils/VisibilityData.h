#pragma once

#include <cstdint>
#include <string.h>
#include <string>
#include <vector>

#include "libs/nlohmann/json.hpp"
#include "plugins/audio/audioPlugin.h"

class VisibilityData {
public:
    enum Condition {
        SHOW_WHEN,
        SHOW_WHEN_NOT,
    } cond
        = SHOW_WHEN;

    struct ContextCondition {
        uint8_t dataId = -1;
        AudioPlugin* plugin = NULL;
        Condition cond;
        bool isVisible = true;
    };

    std::vector<ContextCondition> contextConditions;
    bool visible = true;

    AudioPlugin& (*getPlugin)(std::string name, int16_t track);

    VisibilityData(AudioPlugin& (*getPlugin)(std::string name, int16_t track))
        : getPlugin(getPlugin)
    {
    }

    bool isVisible()
    {
        visible = true;
        for (auto& cond : contextConditions) {
            bool res = *(bool*)cond.plugin->data(cond.dataId);
            if (cond.cond == SHOW_WHEN_NOT) {
                cond.isVisible = res == false;
            } else {
                cond.isVisible = res == true;
            }
            visible = visible && cond.isVisible;
        }
        return visible;
    }

    void init(nlohmann::json& config, int16_t track)
    {
        if (config.contains("visibilityData") && config["visibilityData"].is_array()) {
            for (auto& context : config["visibilityData"]) {
                ContextCondition cond;
                std::string condition = context["condition"].get<std::string>();
                if (condition == "SHOW_WHEN_NOT") {
                    cond.cond = SHOW_WHEN_NOT;
                } else {
                    cond.cond = SHOW_WHEN;
                }

                cond.plugin = &getPlugin(context["plugin"].get<std::string>(), track);
                if (!cond.plugin) {
                    continue;
                }
                cond.dataId = cond.plugin->getDataId(context["dataId"].get<std::string>());
                contextConditions.push_back(cond);
            }
        }
    }
};
