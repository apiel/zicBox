#pragma once

#include "plugins/audio/audioPlugin.h"
#include "plugins/audio/mapping.h"

class SampleEngine : public Mapping {
public:
    float& index;
    struct SampleBuffer {
        uint64_t count = 0;
        float* data;
    };
    SampleBuffer& sampleBuffer;

    std::string name = "Engine";

    SampleEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, SampleBuffer& sampleBuffer, float& index, std::string name)
        : Mapping(props, config)
        , sampleBuffer(sampleBuffer)
        , index(index)
        , name(name)
    {
    }

    virtual float getSample(int index, float stepIncrement)
    {
        return sampleBuffer.data[index];
    }

    virtual void opened(float stepMultiplier) { }

    virtual void serializeJson(nlohmann::json& json) { }
    virtual void hydrateJson(nlohmann::json& json) { }

    virtual float* getIndex() { return NULL; }
};
