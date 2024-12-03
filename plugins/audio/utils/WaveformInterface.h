#ifndef _WAVEFORM_INTERFACE_H_
#define _WAVEFORM_INTERFACE_H_

#include <cstdint>

class WaveformInterface {
public:
    uint64_t sampleCount;

    WaveformInterface(uint64_t sampleCount)
        : sampleCount(sampleCount)
    {
    }

    virtual float sample(float* index, float freq) = 0;
    virtual float* sample(float* index) = 0;
    virtual float* samples() = 0;
};

#endif
