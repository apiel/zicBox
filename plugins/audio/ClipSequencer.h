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

    struct TimelineEvent {
        uint32_t step;
        uint16_t clip;
        uint32_t loop = -1;
    };
    std::vector<TimelineEvent> events;

    void loadTimeline()
    {
    }

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

    void hydrateJson(nlohmann::json& json) override { } // Do not hydrate this plugin
    void serializeJson(nlohmann::json& json) override { }
};
