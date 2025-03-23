#pragma once

#include <cstdint>

class WavetableInterface {
public:
    uint64_t sampleCount;

    WavetableInterface(uint64_t sampleCount)
        : sampleCount(sampleCount)
    {
    }

    virtual float sample(float* index, float freq) = 0;
    virtual float* sample(float* index) = 0;
    virtual float* samples() = 0;
};
