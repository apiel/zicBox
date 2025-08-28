#pragma once

#include "plugins/audio/audioPlugin.h"
#include "plugins/audio/mapping.h"

class SampleEngine : public Mapping {
public:
    struct SampleBuffer {
        uint64_t count = 0;
        float* data;
    };
    SampleBuffer& sampleBuffer;

    std::string name = "Engine";

    SampleEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, SampleBuffer& sampleBuffer, std::string name)
        : Mapping(props, config)
        , sampleBuffer(sampleBuffer)
        , name(name)
    {
    }

    void sample(float* buf) override { }

    virtual void sample(float* buf, int index) { }

    virtual float getSample(int index, float stepIncrement)
    {
        return sampleBuffer.data[index];
    }

    virtual void serializeJson(nlohmann::json& json) { }
    virtual void hydrateJson(nlohmann::json& json) { }
};
