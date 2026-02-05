#pragma once

#include "audio/engines/EngineBase.h"

class DrumKick2 : public EngineBase<DrumKick2> {
public:
    // This stays in RAM (DTCM for H7 speed)
    Param params[2] = {
        { .label = "Decay", .min = 0.0f, .max = 1.0f, .step = 0.01f },
        { .label = "Pitch", .min = 20.0f, .max = 200.0f, .step = 1.0f }
    };

    // No need to pass '2' anymore! The base class template deduces it.
    DrumKick2() : EngineBase(Drum, "Kick2", params) {}

    void noteOnImpl(uint8_t note) { /* ... */ }
    float sampleImpl() { return 0.0f; }
};