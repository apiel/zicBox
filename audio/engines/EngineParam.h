#pragma once

#include <cstdint>

#include "helpers/calculatePrecision.h"

enum ParamType : int8_t {
    VALUE_BASIC = 1 << 0, // 1
    VALUE_CENTERED = 1 << 1, // 2
    VALUE_STRING = 1 << 2 // 4
};

enum ParamIncrementationType : int8_t {
    INC_BASIC = 1 << 0, // 1
    INC_EXP = 1 << 1, // 2
    INC_MULT = 1 << 2, // 4
    INC_ONE_BY_ONE = 1 << 3, // 8
    INC_SCALED = 1 << 4, // 16
};

enum ParamModule : int8_t {
    MODULE_NONE = 0,
    MODULE_OSC_WAVETABLE,
    MODULE_RESONANT_FILTER,
    MODULE_ENV_ADSR,
    MODULE_LFO,
};

struct Param {
    // Flash
    const char* key = nullptr;
    const char* label;
    const char* unit = nullptr;
    // RAM Buffer
    char* string = nullptr;

    float value = 0.0f;
    float min = 0.0f;
    float max = 100.0f;
    float step = 1.00f;

    int8_t incType = INC_BASIC;
    int8_t type = -1;
    int8_t precision = -1;

    int8_t target = 0;
    int8_t module = MODULE_NONE;

    void* context = nullptr;
    void (*onUpdate)(void* ctx, float val) = nullptr;
    float (*graph)(void* ctx, float val) = nullptr;
    void (*hydrateFn)(void* ctx, const char* val) = nullptr;

    const char* description = nullptr;

    void set(float newValue)
    {
        value = (newValue < min) ? min : (newValue > max ? max : newValue);
        if (onUpdate != nullptr) {
            onUpdate(context, value);
        }
    }

    void inc(float delta)
    {
        if (incType & INC_MULT) {
            float mult = (step == 1.0f ? 1.1f : step) * abs(delta);
            if (delta < 0) {
                set(value / mult);
            } else {
                set(value * mult);
            }
            return;
        }

        if (incType & INC_SCALED) {
            float ratio = (step > 1.0f ? 1.0f : step); // used to reduce increments
            if (value >= 10000) {
                set(value + (delta > 0 ? 1000 : -1000) * ratio);
            } else if (value >= 1000) {
                set(value + (delta > 0 ? 100 : -100) * ratio);
            } else if (value >= 100) {
                set(value + (delta > 0 ? 10 : -10) * ratio);
            } else if (value <= -10000) {
                set(value - (delta > 0 ? 1000 : -1000) * ratio);
            } else if (value <= -1000) {
                set(value - (delta > 0 ? 100 : -100) * ratio);
            } else if (value <= -100) {
                set(value - (delta > 0 ? 10 : -10) * ratio);
            } else {
                set(value + delta * ratio);
            }
            return;
        }

        if (incType & INC_ONE_BY_ONE) {
            delta = delta > 0 ? 1 : -1;
        }
        if (incType & INC_EXP) {
            // use step for base
            float base = step == 1.0f ? 2.0f : step;
            float incVal = log(value) / log(base);
            incVal += delta;
            set(pow(base, incVal));
            return;
        }
        set(value + ((float)delta * step));
    }

    float getGraphPoint(float val) { return graph(context, val); }

    void hydrate(const char* val) {
        if (hydrateFn != nullptr) {
            hydrateFn(context, val);
        }
    }

    // Helper to finalize inference on the MCU
    void finalize()
    {
        if (precision == -1) precision = calculatePrecision(step);
        if (type == -1) {
            if (string != nullptr) type = VALUE_STRING;
            else if (min == -max) type = VALUE_CENTERED;
            else type = VALUE_BASIC;
        }
        if (onUpdate != nullptr) {
            onUpdate(context, value);
        }
    }
};
