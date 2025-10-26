#pragma once

#include "plugins/audio/SynthMultiEngine.h"
#include "plugins/audio/utils/lookupTable.h"

#include <cmath>

class Audio {
protected:
    LookupTable lookupTable;

public:
    AudioPlugin::Props props = {
        .sampleRate = 48000,
        .channels = 2,
        .audioPluginHandler = nullptr,
        .maxTracks = MAX_TRACKS,
        .lookupTable = &lookupTable,
    };

    nlohmann::json json = nlohmann::json::object();
    AudioPlugin::Config config = {
        .name = "Audio",
        .json = json,
    };

    SynthMultiEngine multiEngine;

    Audio()
        : multiEngine(props, config)
    {
    }

    // float phase = 0.0f;
    // float sample()
    // {
    //     float out = 0.2f * sinf(phase);
    //     phase += 2.0f * M_PI * 440.0f / props.sampleRate;
    //     if (phase >= 2.0f * M_PI)
    //         phase -= 2.0f * M_PI;
    //     return out;
    // }

    float sample()
    {
        float out;
        multiEngine.sample(&out);
        return out;
    }
};