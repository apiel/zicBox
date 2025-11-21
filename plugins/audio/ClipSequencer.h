#pragma once

#include "Tempo.h"
#include "audioPlugin.h"
#include "log.h"
#include "mapping.h"
#include "plugins/audio/utils/Workspace.h"
#include "stepInterface.h"
#include "plugins/audio/utils/Timeline.h"

/*md
## ClipSequencer
*/

class ClipSequencer : public Mapping, public UseClock {
protected:
    Workspace workspace;
    Timeline timeline;

    AudioPlugin* targetPlugin = NULL;
    uint8_t setClipDataId = 0;

    std::string timelinePath = workspace.getCurrentPath() + "/timeline.json";

    uint32_t stepCounter = -1;
    bool isPlaying = false;

    uint16_t currentEvent = 0;

    void loadTimeline()
    {
        std::ifstream file(timelinePath);
        if (file.is_open()) {
            logDebug("Loading timeline: %s", timelinePath.c_str());
            nlohmann::json json;
            file >> json;
            file.close();

            timeline.load(json);
        } else {
            logWarn("Unable to open timeline file: %s", timelinePath.c_str());
        }
    }

    void onStep() override
    {
        stepCounter++;

        // Handle all events for this step
        while (currentEvent < timeline.events.size() && timeline.events[currentEvent].step == stepCounter) {
            Timeline::Event& event = timeline.events[currentEvent];

            if (event.type == Timeline::EventType::LOOP_BACK) {
                logDebug("Event on step %d loop back to step %d", stepCounter, event.value);
                stepCounter = event.value;

                // find event for the new stepCounter
                currentEvent = 0;
                while (currentEvent < timeline.events.size() && timeline.events[currentEvent].step < stepCounter) {
                    currentEvent++;
                }
            } else if (event.type == Timeline::EventType::LOAD_CLIP) {
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
