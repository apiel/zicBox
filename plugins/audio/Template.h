/** Description:
This code serves as the foundation, or blueprint, for a custom audio processing component, such as an effects unit or a software synthesizer plugin. It defines all the necessary connections and features required for the component to operate within a larger audio environment.

### 1. User Controls and Parameters

The blueprint sets up three specific, user-adjustable parameters (`param1`, `param2`, `param3`). These act as the virtual "knobs" or sliders users interact with:

*   **Definition:** Each parameter is given a default value, a unique name, and precise limits. For example, `param1` is configured to operate like a frequency control, ranging from 10 Hz up to 2000 Hz. `param3` is designed for fine adjustments, measured in percentages.
*   **Accessibility:** The framework handles ensuring these user-set values are correctly passed into the core audio mathematics.

### 2. Core Audio Processing

The fundamental functionality of the plugin centers around how it handles sound over time:

*   **The `sample` Function:** This is the most crucial part. Audio is processed one tiny fraction of a second at a time (a "sample"). The `sample` function is called continuously—thousands of times per second—and this is where the developer inserts the unique code (the "magic") needed to generate a new sound waveform or modify an existing audio signal.
*   **Musical Interaction:** The component is equipped with functions (`noteOn`, `noteOff`) that allow it to respond instantly to real-time musical instructions, such as when a key is pressed or released on a keyboard, making it capable of acting as a musical instrument.

In summary, this definition is the empty vessel for a new audio tool, providing the user interface controls and the essential high-speed processing loop where the sound generation calculations will eventually occur.

sha: 00bc5b631db6a72c03523e792b470353d4613186d0c7306fc7a4adc5a4447c41 
*/
#pragma once

#include "audioPlugin.h"
#include "mapping.h"

class MyAudioPlugin : public Mapping {
protected:
public:
    // Example of params: param1
    Val& param1 = val(200.0f, "PARAM1", { "Param 1", .min = 10.0, .max = 2000.0, .step = 10.0, .unit = "Hz" });
    // Example of params: param2, by default .min = 0.0, .max = 100.0, .step = 1.0
    Val& param2 = val(50.0f, "PARAM2", { "Param 2" });
    // Example of params: param3
    Val& param3 = val(0.0f, "PARAM3", { "Param 3", .min = 0.0, .max = 100.0, .step = 0.1, .unit = "%" });
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
