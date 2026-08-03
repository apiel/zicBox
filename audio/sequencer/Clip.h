#pragma once

#include <string>
#include <vector>

#include "audio/sequencer/Step.h"

struct ParamValue {
    std::string key;
    float value;
    std::string string;
};

struct Clip {
    bool validated = false;
    std::vector<ParamValue> paramValues;
    std::vector<Step> sequence;
    bool saved = false;
    uint8_t engineId = 0;
    std::string name;
    int noteRepeat = 2;
};
