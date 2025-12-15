#pragma once

#include "plugins/audio/Tempo.h"

/*md
## TimelineTempo
*/

class TimelineTempo : public Tempo {
protected:
    uint16_t stepTrack = STEP_TRACK;
    uint32_t stepCounter = -1;

    static constexpr uint32_t STEPS_PER_BAR = 16;

public:
    // LOOP_START is expressed in bars and limited by float precision.
    // Clock counters are sent as float, which preserves only 24 bits of integer precision (~16.7M).
    // With 1 step every 6 clock pulses and 16 steps per bar, this allows ≈174,762 bars max.
    // This cap guarantees exact clock/step/bar values with no drift or skipped events.
    static constexpr float MAX_BARS = 174762.0f;

    Val& loopStart = val(0.0f, "LOOP_START", { "Loop start", .max = MAX_BARS, .unit = "bars" });
    Val& loopLength = val(0.0f, "LOOP_LENGTH", { "Loop length", .max = MAX_BARS, .unit = "bars" });

    TimelineTempo(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Tempo(props, config)
    {
        stepTrack = config.json.value("stepTrack", stepTrack);
    }

    void sample(float* buf) override
    {
        if (!props.audioPluginHandler->isPlaying())
            return;

        uint32_t clockValue = clock.getClock();
        buf[clockTrack] = clockValue;

        if (clockValue == 0)
            return;

        // ---- advance step every 6 clocks ----
        if (clockValue % 6 == 0) {
            stepCounter++;

            // ---- LOOP LOGIC ----
            float loopLenBars = loopLength.get();
            if (loopLenBars > 0.0f) {
                uint32_t loopStartBars = (uint32_t)loopStart.get();
                uint32_t loopLengthBars = (uint32_t)loopLenBars;

                uint32_t loopStartStep = loopStartBars * STEPS_PER_BAR;
                uint32_t loopEndStep = loopStartStep + loopLengthBars * STEPS_PER_BAR;

                if (stepCounter >= loopEndStep) {
                    // logDebug("Looping from %d to %d", loopStartStep, loopEndStep);
                    stepCounter = loopStartStep;
                    props.audioPluginHandler->sendEvent(AudioEventType::TEMPO_LOOP);
                }
            }

            buf[stepTrack] = stepCounter;
        }
    }

    void onEvent(AudioEventType event, bool playing) override
    {
        if (event == AudioEventType::STOP || (event == AudioEventType::TOGGLE_PLAY_STOP && !playing)) {
            stepCounter = 0;
        }
    }
};

class UseTimelineClock : public UseClock {
protected:
    uint32_t stepCounter = -1;

public:
    uint16_t stepTrack = STEP_TRACK;

    void sample(float* buf) override
    {
        stepCounter = (uint32_t)buf[stepTrack];
        UseClock::sample(buf);
    }
};
