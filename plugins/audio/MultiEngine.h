/** Description:
This C++ header defines a foundational component named `MultiEngine`. This component acts primarily as a specialized controller or wrapper for managing parameters within a larger digital audio system, such as a plugin or effect unit.

**Core Purpose and Structure**

The `MultiEngine` is designed to handle the configuration and real-time adjustment of audio processing variables. It inherits characteristics from a generic `Mapping` structure, meaning its essential task is to link abstract inputs (like commands or user interface actions) to concrete parameters used by the audio circuitry.

**How it Works**

When this engine is initialized, it receives necessary settings and properties. Its key feature is a dedicated communication mechanism: a specialized function pointer. Think of this as a private, instant telephone line.

The `setVal` function is the control input. When an external system (like a user interface knob or an automation track) attempts to change a parameter (identified by a text key and a numerical value), the `MultiEngine` immediately uses its stored communication line to inform the main audio application about the change. This ensures that any value change is instantly routed back to the appropriate place for execution.

Finally, the structure includes placeholder functions for JSON serialization. Although currently empty, these methods indicate that the engine is designed to be able to save all its current configuration settings to a file and restore them later, ensuring the state of the audio effect is persistent.

sha: 553d5509805f2c44acb6941ee82958ec73dd4fb5396cc3cd5d44b3d9d1bd384e 
*/
#pragma once

#include "plugins/audio/audioPlugin.h"
#include "plugins/audio/mapping.h"

typedef std::function<void(std::string, float)> SetValFn;

class MultiEngine : public Mapping {
public:
    bool needCopyValues = false;

    std::string name = "Engine";

    SetValFn setValFn = nullptr;

    MultiEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, std::string name)
        : Mapping(props, config)
        , name(name)
    {
    }

    void setVal(std::string key, float value)
    {
        if (setValFn) {
            setValFn(key, value);
        }
    }

    virtual void opened() { }

    virtual void serializeJson(nlohmann::json& json) { }
    virtual void hydrateJson(nlohmann::json& json) { }
};
