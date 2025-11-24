/** Description:
This component, named `Timeline`, acts as a central scheduler, defining a sequence of actions that occur at precise moments within an application, likely for audio or media processing.

At its core, the `Timeline` manages a list of individual `Events`. Each `Event` specifies a time point ("step"), the type of action to perform (such as loading a sound clip or looping back), and any necessary related data.

The component relies on a `Workspace` helper to manage project file paths and determine the exact location of the scheduling configuration, typically stored in a file named `timeline.json`.

The main functions handle data ingestion:
1.  **Configuration:** It sets up the project folder path and identifies the timeline file.
2.  **Loading:** It reads the structured data from the specified file. It then parses this external data, validates that all necessary details are present for each entry, and converts it into the internal list of scheduled `Events`. Crucially, it automatically sorts these events by their time "step" to guarantee they are processed in the correct chronological order during playback. This structure ensures a reliable, predictable schedule for the application.

sha: a14b4bf8145be4ad61cf2992c7a9416403d303d10a6ef5667e21d266967500be 
*/
#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "log.h"
#include "plugins/audio/utils/Workspace.h"

class Timeline {
public:
    Workspace workspace;
    std::string timelinePath = workspace.getCurrentPath() + "/timeline.json";

    enum EventType {
        LOAD_CLIP,
        LOOP_BACK,
    };

    struct Event {
        uint32_t step;
        EventType type;
        uint32_t value;
        void* data = nullptr;
    };

    std::vector<Event> events;

    void load(nlohmann::json& json)
    {
        events.clear();
        events.reserve(json.size()); // not mandatory but improve performance

        for (auto& e : json) {
            if (!e.contains("step") || !e.contains("type") || !e.contains("value")) {
                logWarn("Skipping timeline entry missing step, type or value.");
                continue;
            }

            Event ev;
            ev.step = e["step"];
            ev.value = e["value"];

            std::string type = e["type"];
            if (type == "LOAD_CLIP") ev.type = LOAD_CLIP;
            else if (type == "LOOP_BACK") ev.type = LOOP_BACK;

            events.push_back(ev);
        }

        // Sort timeline by step
        std::sort(events.begin(), events.end(),
            [](auto& a, auto& b) { return a.step < b.step; });
    }

    void config(nlohmann::json& json)
    {
        workspace.folder = json.value("workspaceFolder", workspace.folder);
        workspace.init();

        timelinePath = workspace.getCurrentPath() + "/" + json.value("timelineFilename", "timeline.json");

        load();
    }

    void load()
    {
        std::ifstream file(timelinePath);
        if (file.is_open()) {
            logDebug("Loading timeline: %s", timelinePath.c_str());
            nlohmann::json json;
            file >> json;
            file.close();
            load(json);
        } else {
            logWarn("Unable to open timeline file: %s", timelinePath.c_str());
        }
    }

    void reloadWorkspace()
    {
        workspace.init();
        load();
    }
};
