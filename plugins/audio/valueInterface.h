#ifndef _MAPPING_INTERFACE_H_
#define _MAPPING_INTERFACE_H_

#include <stdint.h>
#include <string>

enum ValueType {
    VALUE_BASIC,
    VALUE_CENTERED,
    VALUE_STRING,
};

enum ValueIncrementationType {
    VALUE_INCREMENTATION_BASIC,
    VALUE_INCREMENTATION_EXP,
    VALUE_INCREMENTATION_MULT,
};

class ValueInterface {
public:
    struct Props {
        std::string label = "";
        ValueType type = VALUE_BASIC;
        float min = 0.0f;
        float max = 100.0f;
        float step = 1.00f;
        uint8_t floatingPoint = 0;
        const char* unit = nullptr;
        ValueIncrementationType incrementationType = VALUE_INCREMENTATION_BASIC;
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
    virtual void onUpdate(void (*callback)(float, void* data), void* data) = 0;
    virtual void checkForUpdate() = 0;
};

#endif