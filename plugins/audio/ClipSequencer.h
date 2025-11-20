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

    AudioPlugin* targetPlugin = NULL;
    uint8_t setClipDataId = 0;

    std::string timelinePath = workspace.getCurrentPath() + "/timeline.json";

    uint32_t stepCounter = -1;
    bool isPlaying = false;

    enum EventType {
        LOAD_CLIP,
        LOOP_BACK,
    };

    struct TimelineEvent {
        uint32_t step;
        EventType type;
        uint32_t value;
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
                if (!entry.contains("step") || !entry.contains("type") || !entry.contains("value")) {
                    logWarn("Skipping timeline entry missing step, type or value.");
                    continue;
                }
                TimelineEvent event;
                event.step = entry["step"].get<uint32_t>();
                event.value = entry["value"].get<uint32_t>();
                std::string type = entry["type"].get<std::string>();
                if (type == "LOAD_CLIP") {
                    event.type = EventType::LOAD_CLIP;
                } else if (type == "LOOP_BACK") {
                    event.type = EventType::LOOP_BACK;
                }
                // logDebug("- Timeline event step %d type %d value %d", event.step, event.type, event.value);
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

        // Handle all events for this step
        while (currentEvent < events.size() && events[currentEvent].step == stepCounter) {
            TimelineEvent& event = events[currentEvent];

            if (event.type == EventType::LOOP_BACK) {
                logDebug("Event on step %d loop back to step %d", stepCounter, event.value);
                stepCounter = event.value;

                // find event for the new stepCounter
                currentEvent = 0;
                while (currentEvent < events.size() && events[currentEvent].step < stepCounter) {
                    currentEvent++;
                }
            } else if (event.type == EventType::LOAD_CLIP) {
                logDebug("Event on step %d clip %d", stepCounter, event.value);
                if (targetPlugin) {
                    targetPlugin->data(setClipDataId, &event.value);
                }
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

        //md - `"target": "pluginName"` the plugin to set clips (serializer...)
        if (json.contains("target")) {
            targetPlugin = &props.audioPluginHandler->getPlugin(json["target"].get<std::string>(), track);
            if (targetPlugin) {
                setClipDataId = targetPlugin->getDataId(json.value("loadClipDataId", "LOAD_CLIP"));
            } else {
                logError("Unable to find target plugin: %s", json["target"].get<std::string>().c_str());
            }
        } else {
            logError("No target plugin specified for ClipSequencer");
        }

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
