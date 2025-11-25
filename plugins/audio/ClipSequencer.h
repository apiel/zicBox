/** Description:
The `ClipSequencer` acts as a specialized timeline manager or conductor within the audio system. Its primary role is to coordinate and execute a predefined sequence of events based on the musical tempo or clock steps.

**Basic Operation:**

1.  **The Clock:** The Sequencer listens to the system's musical timing. Every time a new beat or step arrives, its internal step counter advances.
2.  **The Timeline:** It holds a collection of scheduled actions—the "Timeline." When a step advances, the Sequencer checks this list.
3.  **Execution:** If the current step matches a scheduled event, the Sequencer executes the command.

**Key Functions:**

*   **Loading Clips:** The most common action is telling another designated component (the "Target Plugin") to load a specific audio clip or data set.
*   **Looping:** It can execute flow control commands, such as instantly jumping the internal step counter back to an earlier point in the sequence, allowing for complex musical looping structures.
*   **Control:** It responds to global audio events. For instance, when the music system stops, the Sequencer resets its step counter, preparing to start the sequence from the beginning.

In short, the `ClipSequencer` ensures that specific audio changes (like swapping clips) happen precisely on schedule, acting as the system's central script reader for complex, timed musical structures.

sha: 4b7af627252ec2854e6b4d8e56788a915d50d2b547cb0244f6a914e9f00da3ac 
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
