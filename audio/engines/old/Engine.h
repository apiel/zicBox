/** Description:
This C++ Header file defines the fundamental blueprint for an abstract component named `Engine`. This component acts as the core sound generator within an audio or music application, such as a synthesizer or drum machine.

Since this is an abstract definition, the file establishes the mandatory structure and behavior for all sound generators, but it does not contain the actual sound processing logic itself. Specific sound generators (like a detailed drum or synth) must be built separately based on this template.

**Core Functionality and Structure:**

1.  **Properties:** Every `Engine` must store basic identifying information: a full `name`, a concise `shortName`, and a specific `Type`. The defined types are limited to either `Drum` or `Synth`.
2.  **Audio Integration:** The structure mandates two essential functions for sound generation:
    *   `noteOn`: This is how the system signals the engine to begin playing a specific musical note.
    *   `sample`: This function is required to continuously output the raw pieces of audio data (samples) that the system combines to create the audible sound wave.
3.  **System Capability:** By extending the `Serializable` trait, any concrete sound engine created from this blueprint is inherently capable of having its current state and settings saved to storage and accurately reloaded later.

sha: 75aedcc59721090a539f6ea0582ff93f23ca9d3188c070f99d9611af0cc418e0 
*/
#pragma once

#include <cstdint>
#include <string>

#include "audio/Serializable.h"

class Engine : public Serializable {
public:
    std::string name;
    std::string shortName;

    enum Type {
        Drum,
        Synth,
    } type;

    // Engine(Type type, std::string name)
    //     : Engine(type, name, name)
    // {
    // }

    Engine(Type type, std::string name, std::string shortName)
        : name(name)
        , shortName(shortName)
        , type(type)
    {
    }

    virtual void noteOn(uint8_t note) = 0;
    virtual float sample() = 0;
};
