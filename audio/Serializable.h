#pragma once

#include <string>
#include <variant>
#include <vector>

class Serializable {
public:
    using Value = std::variant<std::string, float>;
    struct KeyValue {
        std::string key;
        Value value;
    };

    virtual void hydrate(const std::vector<KeyValue>& values) = 0;
    virtual std::vector<KeyValue> serialize() const = 0;
};
