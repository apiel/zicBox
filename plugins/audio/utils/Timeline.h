/** Description:
This C++ structure defines a core component called `Timeline`, which acts as the scheduled manager for an application, likely an audio or media program using plugins. Its primary function is to read a configuration file (typically a JSON file) and convert those written instructions into a sequence of timed actions that the program can execute.

**How it Works:**

1.  **Workspace Management:** The `Timeline` first establishes its location using a `Workspace` utility. This sets up the directory structure, defining where the system expects to find related resource files. It then determines the exact path to its instruction script, usually `timeline.json`.

2.  **Defining Actions (Events):** The system defines specific, executable actions, such as `LOAD_CLIP` (start playing a piece of media) or `LOOP_BACK` (return to a previous point in the sequence). These actions are packaged into `Events`. Each `Event` specifies *what* action to take, *when* to take it (the numerical "step"), and associated parameters.

3.  **Loading the Script:** The system includes functions to load the timeline:
    *   A configuration function sets the initial folder and filename, then triggers the loading process.
    *   A loading function reads the specified JSON file from the disk.

4.  **Processing and Sorting:** Once loaded, the system parses the data. It verifies that every instruction is complete and converts the textual action descriptions into internal codes. Most importantly, it immediately sorts all loaded instructions based on their numerical "step," guaranteeing that they are always processed in chronological order.

In essence, the `Timeline` class reads a script describing a series of events and prepares them for execution, ensuring proper file path handling and sequential order.

sha: 69e3eab867ac4d5fa129c0109edea0bdddcd3ad451c39af1156c7470c7f709b8 
*/
#pragma once

#include "libs/nlohmann/json.hpp"
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
