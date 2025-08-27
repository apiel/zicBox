#pragma once

#include "plugins/audio/audioPlugin.h"
#include "plugins/audio/mapping.h"

class SampleEngine : public Mapping {
public:
    struct SampleBuffer {
        uint64_t count = 0;
        float* data;
    };

    std::string name = "Engine";

    SampleEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, std::string name)
        : Mapping(props, config)
        , name(name)
    {
    }

    virtual float getSample(SampleBuffer& sampleBuffer, int index)
    {
        return sampleBuffer.data[index];
    }

    virtual void serializeJson(nlohmann::json& json) { }
    virtual void hydrateJson(nlohmann::json& json) { }
};
