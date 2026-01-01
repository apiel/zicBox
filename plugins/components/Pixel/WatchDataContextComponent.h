#pragma once

#include "log.h"
#include "plugins/components/component.h"

class WatchDataContextComponent : public Component {
protected:
    AudioPlugin* plugin = NULL;

    struct DataContext {
        uint8_t dataId = -1;
        int16_t contextIndex;
        float lastValue = -9999999999.99999;
    };

    std::vector<DataContext> contextConditions;

public:
    WatchDataContextComponent(ComponentInterface::Props props)
        : Component(props)
    {
        nlohmann::json& config = props.config;

        plugin = &getPlugin(config["audioPlugin"].get<std::string>(), track);
        if (!plugin) {
            logError("WatchDataContext, unable to find plugin: %s", config["plugin"].get<std::string>().c_str());
            return;
        }

        if (config.contains("data") && config["data"].is_array()) {
            for (auto& context : config["data"]) {
                DataContext cond;

                cond.dataId = plugin->getDataId(context["dataId"].get<std::string>());
                cond.contextIndex = context["contextIndex"].get<int16_t>();
                contextConditions.push_back(cond);
            }
        }

        jobRendering = [this](unsigned long now) {
            for (auto& cond : contextConditions) {
                float value = *(float*)plugin->data(cond.dataId);
                if (value != cond.lastValue) {
                    cond.lastValue = value;
                    logDebug("WatchDataContext set %d: %f", cond.contextIndex, cond.lastValue);
                    setContext(cond.contextIndex, cond.lastValue);
                }
            }
        };
    }

    void render() override { }
};
