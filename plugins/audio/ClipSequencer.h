#pragma once

#include "Tempo.h"
#include "audioPlugin.h"
#include "log.h"
#include "mapping.h"
#include "plugins/audio/utils/Workspace.h"
#include "stepInterface.h"

/*md
## ClipSequencer
*/

class ClipSequencer : public Mapping, public UseClock {
protected:
    Workspace workspace;

    std::string timelinePath = workspace.getCurrentPath() + "/timeline.json";

    uint32_t stepCounter = -1;
    bool isPlaying = false;

    struct TimelineEvent {
        uint32_t step;
        uint16_t clip;
        uint32_t loop = -1;
    };
    std::vector<TimelineEvent> events;
    uint16_t currentEvent = 0;

    void loadTimeline()
    {
        std::ifstream file(timelinePath);
        if (file.is_open()) {
            logDebug("Loading timeline: %s", timelinePath.c_str());
            nlohmann::json json;
            file >> json;
            file.close();

            events.clear();
            events.reserve(json.size());

            for (auto& entry : json) {
                if (!entry.contains("step") || !entry.contains("clip")) {
                    logWarn("Skipping timeline entry missing step or clip.");
                    continue;
                }
                TimelineEvent event;
                event.step = entry["step"].get<uint32_t>();
                event.clip = entry["clip"].get<uint16_t>();
                event.loop = entry.value("loop", UINT32_MAX); // optional
                events.push_back(event);
            }

            // 🔥 Sort timeline by step
            std::sort(events.begin(), events.end(),
                [](const TimelineEvent& a, const TimelineEvent& b) {
                    return a.step < b.step;
                });
        } else {
            logWarn("Unable to open timeline file: %s", timelinePath.c_str());
        }
    }

    void onStep() override
    {
        stepCounter++;
        if (currentEvent < events.size()) {
            TimelineEvent& event = events[currentEvent];
            if (event.step == stepCounter) {
                logDebug("Event on step %d clip %d", stepCounter, event.clip);
                currentEvent++;
            }
        }
    }

public:
    ClipSequencer(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        //md **Config**:
        auto& json = config.json;

        //md - `"workspaceFolder": "data/workspaces"` to set workspace folder.
        workspace.folder = json.value("workspaceFolder", workspace.folder);
        workspace.init();

        //md - `"filename": "timeline"` to set filename. By default it is `timeline.json`.
        timelinePath = workspace.getCurrentPath() + "/" + json.value("filename", "timeline.json");
        loadTimeline();
    }

    void sample(float* buf) override
    {
        UseClock::sample(buf);
    }

    void onEvent(AudioEventType event, bool playing) override
    {
        isPlaying = playing;
        if (event == AudioEventType::STOP) {
            stepCounter = 0;
        } else if (event == AudioEventType::RELOAD_WORKSPACE) {
            workspace.init();
        }
    }

    void hydrateJson(nlohmann::json& json) override { } // Do not hydrate this plugin
    void serializeJson(nlohmann::json& json) override { }
};
