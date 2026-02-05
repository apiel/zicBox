#pragma once

#include <cstdint>

#include "helpers/calculatePrecision.h"

enum ParamType {
    VALUE_BASIC = 1 << 0, // 1
    VALUE_CENTERED = 1 << 1, // 2
    VALUE_STRING = 1 << 2 // 4
};

enum ParamIncrementationType {
    INC_BASIC = 1 << 0, // 1
    INC_EXP = 1 << 1, // 2
    INC_MULT = 1 << 2, // 4
    INC_ONE_BY_ONE = 1 << 3, // 8
    INC_SCALED = 1 << 4, // 16
};

struct Param {
    const char* label; // Flash
    char* unit = nullptr; // RAM Buffer
    char* string = nullptr; // RAM Buffer

    float value = 0.0f;
    float min = 0.0f;
    float max = 100.0f;
    float step = 1.00f;

    int8_t incType = INC_BASIC;
    int8_t type = -1;
    int8_t precision = -1;

    // Helper to finalize inference on the MCU
    void finalize()
    {
        if (precision == -1) precision = calculatePrecision(step);
        if (type == -1) {
            if (string != nullptr) type = VALUE_STRING;
            else if (min == -max) type = VALUE_CENTERED;
            else type = VALUE_BASIC;
        }
    }
};
