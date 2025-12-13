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
        Tempo::sample(buf);
        uint32_t clockValue = (uint32_t)buf[clockTrack];
        if (clockValue != 0) {
            if (clockValue % 6 == 0) {
                stepCounter++;
                buf[stepTrack] = stepCounter;
            }
        }
    }
};

class UseTimelineClock : public UseClock {
protected:
    uint32_t stepCounter = -1;

public:
    uint16_t stepTrack = STEP_TRACK;

    void sample(float* buf)
    {
        UseClock::sample(buf);
        stepCounter = (uint32_t)buf[stepTrack];
    }

    void onStep() override{ onStep(stepCounter); }

    virtual void onStep(uint32_t step) { }
};
