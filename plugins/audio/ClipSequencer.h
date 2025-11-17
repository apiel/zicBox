#pragma once

#include "Tempo.h"
#include "audioPlugin.h"
#include "log.h"
#include "mapping.h"
#include "stepInterface.h"

class ClipSequencer : public Mapping, public UseClock {
protected:
    AudioPlugin::Props& props;

    uint32_t stepCounter = 0;
    bool isPlaying = false;

    AudioPlugin* targetPlugin = NULL;

    void onStep() override
    {
        stepCounter++;
    }

public:
    void sample(float* buf) override
    {
        UseClock::sample(buf);
    }

    void onEvent(AudioEventType event, bool playing) override
    {
        isPlaying = playing;
        if (event == AudioEventType::STOP) {
            stepCounter = 0;
        }
    }
};
