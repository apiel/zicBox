#pragma once

#include <cstdint>
#include <string>

class Engine {
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
        : type(type)
        , name(name)
        , shortName(shortName)
    {
    }

    virtual void noteOn(uint8_t note) = 0;
    virtual float sample() = 0;
};
