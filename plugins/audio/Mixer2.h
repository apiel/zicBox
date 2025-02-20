#pragma once

#include "audioPlugin.h"
#include "mapping.h"

class Mixer2 : public Mapping {
public:
    Val& mix = val(50.0f, "MIX", { "Mix", .type = VALUE_CENTERED });
    uint8_t trackA = 1;
    uint8_t trackB = 2;

    Mixer2(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        trackA = config.json.value("trackA", trackA);
        trackB = config.json.value("trackB", trackB);
        mix.set(config.json.value("mix", mix.get()));
    }

    std::set<uint8_t> trackDependencies() override
    {
        return { trackA, trackB };
    }

    void sample(float* buf)
    {
        buf[track] = buf[trackA] * (1.0f - mix.pct()) + buf[trackB] * mix.pct();
    }
};
