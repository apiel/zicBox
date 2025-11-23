#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "log.h"
#include "plugins/audio/utils/Workspace.h"

class Clip {
public:
    Workspace workspace;

    std::string clipFolder = "track";

protected:
    std::string currentPath = workspace.getCurrentPath() + "/" + clipFolder + "/current.cfg";

public:
    std::string current = "0.json";

    std::string getFilename(int16_t id)
    {
        return std::to_string(id) + ".json";
    }

    std::string getFilepath(std::string file)
    {
        return workspace.getCurrentPath() + "/" + clipFolder + "/" + file;
    }

    std::string getFilepath(int16_t id)
    {
        return getFilepath(getFilename(id));
    }

    void setCurrent(int16_t id)
    {
        // logDebug("set current clip %d in %s", id, currentPath.c_str());
        std::ofstream file(currentPath);
        if (file.is_open()) {
            current = getFilename(id);
            file << current;
            file.close();
        }
    }

    void loadCurrent()
    {
        std::ifstream file(currentPath);
        if (file.is_open()) {
            file >> current;
            file.close();
        } else {
            logDebug("No current clip file: %s", currentPath.c_str());
        }
    }

    void config(nlohmann::json& json)
    {
        workspace.folder = json.value("workspaceFolder", workspace.folder);
        workspace.init();

        clipFolder = json.value("clipFolder", clipFolder);

        init();
    }

    void init()
    {
        workspace.init();
        currentPath = workspace.getCurrentPath() + "/" + clipFolder + "/current.cfg";
        loadCurrent();
    }

    void serialize(nlohmann::json& json, std::string filename)
    {
        std::string path = getFilepath(filename);
        std::ofstream file(path);
        if (file.is_open()) {
            logDebug("Saving clip: %s", path.c_str());
            file << json.dump(4);
            file.close();
        }
    }
    void serialize(nlohmann::json& json) { serialize(json, current); }

    nlohmann::json hydrate(std::string filename)
    {
        // logDebug(">>> Hydrating clip: %s", filename.c_str());
        std::string path = getFilepath(filename);
        std::ifstream file(path);
        if (file.is_open()) {
            // logDebug("Loading clip: %s", path.c_str());
            nlohmann::json json;
            file >> json;
            file.close();
            return json;
        } else {
            logWarn("Unable to open clip file: %s", path.c_str());
            return nlohmann::json();
        }
    }
    nlohmann::json hydrate() { 
        // logDebug(">>> Hydrating current clip: %s", current.c_str());
        return hydrate(current); 
    }
};
