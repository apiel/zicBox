#pragma once

#include <algorithm>
#include <cmath>

class Mixer {
public:
    float kickVol = 0.85f;      // 0.0 to 1.0
    float synth1Vol = 0.70f;    // 0.0 to 1.0
    float synth2Vol = 0.60f;    // 0.0 to 1.0
    float masterDrive = 0.20f;  // 0.0 to 1.0 (Saturation / Fold)

    float processMaster(float inSample)
    {
        if (masterDrive <= 0.0f) return std::clamp(inSample, -1.0f, 1.0f);

        // Warm master overdrive saturation stage
        float gain = 1.0f + (masterDrive * 2.5f);
        float driven = std::tanh(inSample * gain);
        return driven;
    }
};
