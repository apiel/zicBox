/** Description:
This component, the `TimelineSequencer`, functions as a sophisticated timing and scheduling manager within a larger audio system. Think of it as a conductor following a pre-written score, ensuring all musical events occur exactly when they should.

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
## TimelineSequencer
*/

class TimelineSequencer : public Mapping, public UseClock {
protected:
    Timeline timeline;

    AudioPlugin* targetPlugin = NULL;
    uint8_t setClipDataId = 0;

    uint32_t stepCounter = -1;
    bool isPlaying = false;

    uint16_t nextEvent = 0;
    Timeline::Event* timelineEvent = nullptr;

    void onStep() override
    {
        stepCounter++;
        loadNextEvent();
    }

    void loadNextEvent()
    {
        if (nextEvent < timeline.events.size() && timeline.events[nextEvent].step == stepCounter) {

            timelineEvent = &timeline.events[nextEvent];
            logDebug("Event on step %d clip %d", stepCounter, timelineEvent->clip);
            if (targetPlugin) {
                targetPlugin->data(setClipDataId, &timelineEvent->clip);
            }
            nextEvent++;
        }
    }

public:
    TimelineSequencer(AudioPlugin::Props& props, AudioPlugin::Config& config)
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
            logError("No target plugin specified for TimelineSequencer");
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
        } else if (event == AudioEventType::SEQ_LOOP) {
            if (timelineEvent && timelineEvent->gotoStep != -1) {
                logDebug("Event loop back to step %d", timelineEvent->gotoStep);
                stepCounter = timelineEvent->gotoStep;

                // find event for the new stepCounter
                nextEvent = 0;
                while (nextEvent < timeline.events.size() && timeline.events[nextEvent].step < stepCounter) {
                    nextEvent++;
                }
                loadNextEvent();
            }
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
