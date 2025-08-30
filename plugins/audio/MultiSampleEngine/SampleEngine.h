#pragma once

#include "plugins/audio/audioPlugin.h"
#include "plugins/audio/mapping.h"

class SampleEngine : public Mapping {
public:
    float& index;
    float& stepMultiplier;
    struct SampleBuffer {
        uint64_t count = 0;
        float* data;
    };
    SampleBuffer& sampleBuffer;

    std::string name = "Engine";

    SampleEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, SampleBuffer& sampleBuffer, float& index, float& stepMultiplier, std::string name)
        : Mapping(props, config)
        , sampleBuffer(sampleBuffer)
        , index(index)
        , stepMultiplier(stepMultiplier)
        , name(name)
    {
    }

    virtual void opened() { }

    virtual void serializeJson(nlohmann::json& json) { }
    virtual void hydrateJson(nlohmann::json& json) { }
};
