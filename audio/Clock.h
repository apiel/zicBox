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

    // At 250 BPM: increments per second = 250 * 24 / 60 = 100 clock ticks/sec
    // with uint32_t max = 4,294,967,295
    // lasts ~1.36 years of continuous run at 250 BPM.
    uint32_t getClock()
    {
        sampleCounter++;
        if (sampleCounter >= sampleCountTarget) {
            sampleCounter = 0;
            clockCounter++;
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