#pragma once

#include <cstdint>

#include "helpers/clamp.h"

class Clock {
protected:
    int sampleRate;
    uint8_t channels = 2;

    uint8_t bpm = 120;

    uint32_t sampleCounter = 0;
    uint32_t sampleCountTarget = 0;

    uint32_t clockCounter = 0;

public:
    // Val& bpm = val(120.0f, "BPM", { "Bpm", .min = 60.0f, .max = 240.0f }, [&](auto p) { setBpm(p.value); });

    Clock(int sampleRate, uint8_t channels)
        : sampleRate(sampleRate)
        , channels(channels)
    {
        setBpm(bpm);
    }

    // Clock events are sent at a rate of 24 pulses per quarter note
    // The is 4 beats per quarter
    // (24/4 = 6)
    void setBpm(float value)
    {
        bpm = CLAMP(value, 50.0f, 250.0f);
        sampleCountTarget = (uint32_t)(((float)sampleRate * 60.0f / bpm) / 24.0f) * channels;
        // logDebug("Tempo: %d bpm (sample rate: %d, sample count: %d)", (int)bpm, sampleRate, sampleCountTarget);
    }

    uint8_t getBpm() { return bpm; }

    // 6 clock ticks per beat
    // we want 4096 because it is multiple of 4, 8, 16, 32, 64, 128, ...
    // and we can do stuff like 4096 % 32 == 0 to know if we reached the end of a pattern.
    const uint32_t endClockCounter = 4096 * 6;
    uint32_t getClock()
    {
        sampleCounter++;
        if (sampleCounter >= sampleCountTarget) {
            sampleCounter = 0;
            clockCounter = clockCounter + 1.0;
            if (clockCounter > endClockCounter) {
                clockCounter = 1.0;
            }
            return clockCounter;
        }
        return 0;
    }

    void reset()
    {
        sampleCounter = 0;
        clockCounter = 0;
    }
};