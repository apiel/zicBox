#pragma once

#include "plugins/audio/audioPlugin.h"
#include "plugins/audio/mapping.h"

typedef std::function<void(std::string, float)> SetValFn;

class DrumEngine : public Mapping {
public:
    std::string name = "Engine";

    SetValFn setValFn = nullptr;

    DrumEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, std::string name)
        : Mapping(props, config)
        , name(name)
    {
    }

    void setVal(std::string key, float value)
    {
        if (setValFn)
            setValFn(key, value);
    }

    void sample(float* buf) override
    {
    }

    virtual void sampleOn(float* buf, float envAmp, int sampleCounter, int totalSamples) = 0;
    virtual void sampleOff(float* buf) { }

    virtual void serializeJson(nlohmann::json& json) { }
    virtual void hydrateJson(nlohmann::json& json) { }
};
