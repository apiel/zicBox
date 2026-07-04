/** Description:
This code defines a core component called `SampleEngine`, which is essential for managing and processing audio samples within a larger audio plugin system.

**Basic Idea of How It Works:**

The `SampleEngine` acts as a specialized control module. It doesn't physically store large audio files; instead, it manages crucial parameters and links directly to the raw audio data held elsewhere.

1.  **Data Linkage:** It holds connections (references) to the critical variables needed for audio playback, such as the current position within the audio file (`index`), the speed of playback (`stepMultiplier`), and the container holding the actual sound data (`SampleBuffer`). This efficiency ensures the engine operates on live, external data.
2.  **Configuration:** It inherits functionality from a base structure called `Mapping`, indicating that it follows predefined rules for setting up or transforming audio signals. It also includes configuration flags to manage how its settings are copied or saved.
3.  **Lifecycle Management:** The engine includes dedicated slots for system management:
    *   `opened`: A function that is called when the engine is initialized and ready to run.
    *   `serializeJson` and `hydrateJson`: These features handle saving the engine's current settings (like the step multiplier) into a standardized format and loading those settings back, allowing users to recall custom configurations.

In essence, the `SampleEngine` provides the structural backbone necessary to handle, control, and save the settings related to playing back a specific segment of audio within a professional application.

sha: 0843993c48116370b36488fc7496b9d52cf19a35c17f3070c4aa123e6b256fd5 
*/
#pragma once

#include "plugins/audio/audioPlugin.h"
#include "plugins/audio/mapping.h"
#include "plugins/audio/MultiEngine.h"

class SampleEngine : public MultiEngine {
public:
    float& index;
    float& stepMultiplier;
    struct SampleBuffer {
        uint64_t count = 0;
        float* data;
    };
    SampleBuffer& sampleBuffer;


    SampleEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, SampleBuffer& sampleBuffer, float& index, float& stepMultiplier, std::string name)
        : MultiEngine(props, config, name)
        , sampleBuffer(sampleBuffer)
        , index(index)
        , stepMultiplier(stepMultiplier)
    {
    }

    virtual void serializeJson(nlohmann::json& json) { }
    virtual void hydrateJson(nlohmann::json& json) { }
};
