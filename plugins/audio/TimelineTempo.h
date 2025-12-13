#pragma once

#include "plugins/audio/Tempo.h"

/*md
## TimelineTempo
*/

class TimelineTempo : public Tempo {
protected:
    uint16_t stepTrack = STEP_TRACK;
    uint32_t stepCounter = -1;

public:
    TimelineTempo(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Tempo(props, config)
    {
        stepTrack = config.json.value("stepTrack", stepTrack);
    }

    void sample(float* buf) override
    {
        // Tempo::sample(buf);
        if (props.audioPluginHandler->isPlaying()) {
            uint32_t clockValue = clock.getClock();
            buf[clockTrack] = clockValue;
            if (clockValue != 0) {
                if (clockValue % 6 == 0) {
                    stepCounter++;
                    // logDebug("Step timeline: %d", stepCounter);
                    buf[stepTrack] = stepCounter;
                }
            }
        }
    }

    void onEvent(AudioEventType event, bool playing) override
    {
        if (event == AudioEventType::STOP || (event == AudioEventType::TOGGLE_PLAY_STOP && !playing)) {
            logTrace("in timeline tempo event STOP");
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

    // void onStep() override { onStep(stepCounter); }

    // virtual void onStep(uint32_t step) { }
};
