#pragma once

#include "plugins/audio/audioPlugin.h"
#include "plugins/audio/mapping.h"

class SampleEngine : public Mapping {
public:
    std::string name = "Engine";

    SampleEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, std::string name)
        : Mapping(props, config)
        , name(name)
    {
    }

    virtual void serializeJson(nlohmann::json& json) { }
    virtual void hydrateJson(nlohmann::json& json) { }
};
