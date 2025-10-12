#ifndef _MAPPING_INTERFACE_H_
#define _MAPPING_INTERFACE_H_

#include <functional>
#include <stdint.h>
#include <string>

enum ValueType {
    VALUE_BASIC = 1 << 0, // 1
    VALUE_CENTERED = 1 << 1, // 2
    VALUE_STRING = 1 << 2 // 4
};

enum ValueIncrementationType {
    INC_BASIC = 1 << 0, // 1
    INC_EXP = 1 << 1, // 2
    INC_MULT = 1 << 2, // 4
    INC_ONE_BY_ONE = 1 << 3, // 8
    INC_SCALED = 1 << 4, // 16
};

typedef std::function<float (float)> GraphPointFn;

class ValueInterface {
public:
    struct Props {
        std::string label = "";
        uint8_t type = VALUE_BASIC;
        float min = 0.0f;
        float max = 100.0f;
        float step = 1.00f;
        uint8_t floatingPoint = 0;
        std::string unit = "";
        uint8_t incType = INC_BASIC;
        GraphPointFn graph = NULL;
    };

    virtual std::string key() = 0;
    virtual Props& props() = 0;
    virtual std::string label() = 0;
    virtual inline float get() = 0;
    virtual void increment(int8_t steps) = 0;
    virtual inline float pct() = 0;
    virtual void setPct(float pct) = 0;
    virtual std::string string() = 0;
    virtual void set(float value, void* data = NULL) = 0;
    virtual void setOnUpdateCallback(std::function<void(float, void*)> callback, void* data) = 0;
    virtual void checkForUpdate() = 0;
    virtual void copy(ValueInterface* val) { };

    bool hasType(ValueType type)
    {
        return (props().type & type) != 0;
    }

    bool hasAllTypes(uint8_t types)
    {
        return (props().type & types) == types;
    }
};

#endif