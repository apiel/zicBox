/** Description:
This document defines a core timing structure, often called a `Clock`, essential for digital audio and music applications like synthesizers or sequencers.

Its primary goal is to translate musical tempo, measured in Beats Per Minute (BPM), into precise, sample-accurate digital pulses, ensuring perfect rhythmic synchronization within the computer system.

**How it Works:**

The system initializes itself based on the audio engine’s processing speed (the "sample rate"). When a user sets the BPM, the `Clock` performs a critical calculation: it determines the exact number of minuscule audio segments (samples) that must pass before a standardized timing pulse, or "clock tick," should be emitted.

This timing uses a standard highly accurate musical reference: 24 pulses are generated for every single quarter note. This level of granularity ensures that all sequenced events remain perfectly synchronized regardless of the chosen tempo.

The `Clock`’s main function involves constantly counting the incoming audio samples. Once the count reaches the pre-calculated target for the current tempo, the class issues a clock tick, resets its internal sample counter, and waits for the next event. These continuously generated ticks provide the stable, rhythmic backbone needed to trigger musical events.

The internal mechanism is designed for extreme longevity, capable of running continuously for over a year even at maximum tempo settings. Users can dynamically change the tempo within a safe range, and the class provides simple functions to reset the timing counters back to zero.

sha: 5192956bb17ac61171c0ecc76d050ffeedde1b8c53d2b6109b104d03007add5c 
*/
#pragma once

#include <cstdint>

#include "helpers/clamp.h"

class Clock {
protected:
    int sampleRate;

    uint8_t bpm = 120;

    uint32_t sampleCounter = 0;
    uint32_t sampleCountTarget = 0;

    uint32_t clockCounter = 0;

public:
    Clock(int sampleRate)
        : sampleRate(sampleRate)
    {
        setBpm(bpm);
    }

    // Clock events are sent at a rate of 24 pulses per quarter note
    // The is 4 beats per quarter
    // (24/4 = 6)
    void setBpm(float value)
    {
        bpm = CLAMP(value, 50.0f, 250.0f);
        sampleCountTarget = (uint32_t)(((float)sampleRate * 60.0f / bpm) / 24.0f);
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