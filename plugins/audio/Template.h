#pragma once

#include "audioPlugin.h"
#include "mapping.h"
#include "audio/utils.h"

class MyAudioPlugin : public Mapping {
protected:
public:
    // Example of params: param1
    Val& param1 = val(200.0f, "PARAM1", { "Param 1", .min = 10.0, .max = 2000.0, .step = 10.0, .unit = "Hz" });
    // Example of params: param2, by default .min = 0.0, .max = 100.0, .step = 1.0
    Val& param2 = val(50.0f, "PARAM2", { "Param 2" });
    // Example of params: param3
    Val& param3 = val(0.0f, "PARAM3", { "Param 3", .min = 0.0, .max = 100.0, .step = 0.1, .floatingPoint = 1, .unit = "%" });
    // params value can be accessed using .get() for absolute value and .pct() for percentage
    // e.g: param2.get() would be 50, param2.pct() would be 0.5

    MyAudioPlugin(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
    }

    // Audio plugin is called once per sample
    // buf pointer contains all the track: buf[MAX_TRACKS]
    // however, most of the time, only one track is used (unless it is a mixer or something like that)
    void sample(float* buf) override
    {
        buf[track] = 0.0f;
        // instead of returning 0.0f, it should do some magic to generate audio sample...
        // e.g:
        // float output = rand() / (float)RAND_MAX;
        // buf[track] = output;
    }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        // You can access sample rate with props.sampleRate
    }

    void noteOff(uint8_t note, float _velocity, void* userdata = NULL) override
    {
    }
};
