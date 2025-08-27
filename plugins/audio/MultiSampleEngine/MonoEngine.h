#pragma once

#include "plugins/audio/MultiSampleEngine/SampleEngine.h"
#include "plugins/audio/mapping.h"

class MonoEngine : public SampleEngine {
public:
    MonoEngine(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : SampleEngine(props, config, "Mono")
    {
    }

    void sample(float* buf) override { }
};
