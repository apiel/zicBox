#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

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
        : name(name)
        , shortName(shortName)
        , type(type)
    {
    }

    virtual void noteOn(uint8_t note) = 0;
    virtual float sample() = 0;

    using Value = std::variant<std::string, float>;
    struct KeyValue {
        std::string key;
        Value value;
    };
    virtual void hydrate(std::vector<KeyValue> values) = 0;
    virtual std::vector<KeyValue> serialize() = 0;
};
