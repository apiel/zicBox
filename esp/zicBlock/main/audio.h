#pragma once

#include <cmath>

class Audio {
public:
    float phase = 0.0f;
    float freq = 440.0f; // A4
    float sampleRate = 48000.0f;

    float sample()
    {
        float out = 0.2f * sinf(phase);
        phase += 2.0f * M_PI * freq / sampleRate;
        if (phase >= 2.0f * M_PI)
            phase -= 2.0f * M_PI;
        return out;
    }
};