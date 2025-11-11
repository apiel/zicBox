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
