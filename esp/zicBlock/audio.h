#pragma once

#define SKIP_SNDFILE

#include "plugins/audio/SynthMultiEngine.h"
// #include "plugins/audio/MultiDrumEngine/KickEngine.h"
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
        .maxTracks = 1,
        .lookupTable = &lookupTable,
    };

    nlohmann::json json = nlohmann::json::object();
    AudioPlugin::Config config = {
        .name = "Audio",
        .json = json,
    };

    SynthMultiEngine multiEngine;
    // KickEngine multiEngine;

    Audio()
        : multiEngine(props, config)
    {
    }

    float sample()
    {
        float out;
        multiEngine.sample(&out);
        return out;
    }
};