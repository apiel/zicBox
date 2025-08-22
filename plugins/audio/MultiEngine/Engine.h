#pragma once

#include "plugins/audio/audioPlugin.h"
#include "plugins/audio/mapping.h"

class Engine : public Mapping {
public:
    std::string name = "Engine";

    Engine(AudioPlugin::Props& props, AudioPlugin::Config& config, std::string name)
        : Mapping(props, config)
        , name(name)
    {
    }

    void sample(float* buf) override
    {
    }

    virtual void sample(float* buf, float envAmp) = 0;

    virtual void serializeJson(nlohmann::json& json) { }
    virtual void hydrateJson(nlohmann::json& json) { }
};
