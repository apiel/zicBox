/** Description:
This component, the `ClipSequencer`, functions as a sophisticated timing and scheduling manager within a larger audio system. Think of it as a conductor following a pre-written score, ensuring all musical events occur exactly when they should.

**Core Functionality:**
The Sequencer relies on an internal "Timeline," which is a list detailing scheduled actions tied to specific musical steps (like beats or subdivisions). It uses the system’s musical clock to count these steps.

**How it Works:**
1.  **Step Tracking:** As the clock advances, the Sequencer increments its step counter.
2.  **Event Triggering:** At every new step, it checks the Timeline. If an action is scheduled, it is executed immediately.
3.  **Main Action:** The primary action is usually triggering a "Load Clip" command. This sends specific pattern data (the "clip") to an assigned audio component (the "Target Plugin").
4.  **Looping:** It can also handle complex flow control, such as automatically jumping back to an earlier step in the Timeline to create seamless loops or repeating sections.

**Setup and Control:**
The Sequencer must be configured with the name of the "Target Plugin" it needs to communicate with. Once configured, it manages the automatic switching of clips in that target plugin according to its Timeline. It responds to standard musical controls (like Start and Stop) and offers internal control interfaces for viewing or manually triggering specific events.

Tags: C++, Header, Class Definition, Audio Plugin, Sequencer, Timeline Management, Step Synchronization, Event Management, DAW Component, Mapping Interface, Audio Plugin Controller, JSON Configuration, Modular Audio System.
sha: eae9252e401870dedff227558ee4976c19a5904f1758ba8d09bced0eddf9ce4e 
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

    DataFn dataFunctions[2] = {
        { "TIMELINE", [this](void* userdata) {
             return &timeline;
         } },
        { "LOAD_CLIP", [this](void* userdata) {
             int* id = (int*)userdata;
             if (targetPlugin) {
                 targetPlugin->data(setClipDataId, id);
             }
             return nullptr;
         } },
    };
    DEFINE_GETDATAID_AND_DATA
};
