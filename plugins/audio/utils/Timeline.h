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
