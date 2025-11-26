/** Description:
This C++ Header defines a structure named `Clip`, designed to manage the configuration and file handling for individual digital assets or tracks within a larger software project, often used in audio or media applications.

It acts as a specialized file manager, focusing on finding, saving, and loading specific clip data files. The system first relies on a primary "Workspace" object to establish the root location of the project on the computer.

Within this primary location, the `Clip` object uses a dedicated subfolder (by default, named "track") where all individual clip files are stored. These clip files use a standardized, structured data format (JSON) for organization.

A crucial feature is managing the "current" clip. The class maintains a special configuration file (`current.cfg`) which simply stores the filename of the clip that the user is currently working with. Methods are provided to update this configuration (to select a new active clip) and to read it back upon startup (to automatically load the last used clip).

The core functions include: calculating the exact file path for any given clip identifier; saving structured data into a clip file (a process called "serialization"); and loading the content from a clip file back into the program (known as "hydration"). This ensures that all track configurations and data are consistently stored, retrieved, and switched between, regardless of the project's location.

sha: 34e6787528ad68a7f1ffae6845246461460acc346f0e1bdbdd45ff24bdfbeb9a 
*/
#pragma once

#include "libs/nlohmann/json.hpp"
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
