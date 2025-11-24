/** Description:
The `ClipSequencer` serves as a core rhythmic scheduler or conductor within an audio processing system. Its fundamental purpose is to manage the flow of pre-defined actions over time, primarily focused on loading audio segments or configurations, known as "clips."

**Mechanism:**

1.  **Timing:** It relies on an internal musical clock to track steps (like beats or measures).
2.  **Instructions:** It uses a structured *Timeline* object, which is essentially a detailed sequence sheet defining exactly what action should happen at which specific musical step.
3.  **Control:** It is designed to control a separate audio processing unit (called the "target plugin").

**Operation:**

As the music plays, the Sequencer increments a step counter. At every new step, it checks the Timeline for corresponding events. If an event is scheduled (e.g., "load Clip 5"), the Sequencer sends a direct command to the target plugin, triggering the clip change immediately.

The Sequencer also manages flow control within the sequence, specifically handling commands to instantly jump back to an earlier step (a "loop back"), allowing for repeating patterns within the sequence.

**Setup:**

Upon initialization, it reads its configuration to identify precisely which audio device it needs to control and communicates with that device to ensure the sequenced clips are correctly loaded when commanded. It also manages global play/stop events, resetting its internal step counter when the system stops.

sha: 6380f37921c9140540c478f09fc4d2067b428f235c9d300d43f556ec821e2aeb
*/
#pragma once

#include "Tempo.h"
#include "audioPlugin.h"
#include "log.h"
#include "mapping.h"
#include "plugins/audio/utils/Timeline.h"
#include "stepInterface.h"

/*md
## ClipSequencer
*/

class ClipSequencer : public Mapping, public UseClock {
protected:
    Timeline timeline;

    AudioPlugin* targetPlugin = NULL;
    uint8_t setClipDataId = 0;

    uint32_t stepCounter = -1;
    bool isPlaying = false;

    uint16_t currentEvent = 0;

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

        timeline.config(json);

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
            timeline.reloadWorkspace();
        }
    }

    void hydrateJson(nlohmann::json& json) override { } // Do not hydrate this plugin
    void serializeJson(nlohmann::json& json) override { }

    DataFn dataFunctions[1] = {
        { "TIMELINE", [this](void* userdata) {
             return &timeline;
         } },
    };
    DEFINE_GETDATAID_AND_DATA
};
