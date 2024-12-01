#ifndef _WAVEFORM_INTERFACE_H_
#define _WAVEFORM_INTERFACE_H_

#include <cstdint>

class WaveformInterface {
public:
    uint64_t sampleCount = 2048;
    virtual float sample(float* index, float freq) = 0;
    virtual float* samples() = 0;
};

#endif
