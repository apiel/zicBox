#ifndef _MAPPING_INTERFACE_H_
#define _MAPPING_INTERFACE_H_

#include <stdint.h>
#include <string>

enum ValueType {
    VALUE_BASIC,
    VALUE_CENTERED,
    VALUE_STRING,
};

class ValueInterface {
public:
    struct Props {
        const char* label = nullptr;
        ValueType type = VALUE_BASIC;
        float min = 0.0f;
        float max = 100.0f;
        float step = 1.00f;
        uint8_t floatingPoint = 0;
        const char* unit = nullptr;
    };

    virtual const char* key() = 0;
    virtual Props& props() = 0;
    virtual const char* label() = 0;
    virtual inline float get() = 0;
    virtual void increment(int8_t steps) = 0;
    virtual inline float pct() = 0;
    virtual std::string string() = 0;
    virtual void set(float value) = 0;
    virtual void onUpdate(void (*callback)(float, void* data), void* data) = 0;
};

#endif