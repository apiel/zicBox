/** Description:
### Clip Configuration Manager

This component acts as a specialized file handler responsible for organizing, saving, and loading configuration data for individual "Clips." Clips are likely discrete segments of media or audio within a larger project.

**Core Role and Structure:**
The manager’s primary function is to handle the complex relationship between the clip data and the file system. It relies on a "Workspace" utility, which provides the root directory for the entire project. Within this workspace, the manager defines a specific folder (the `clipFolder`) where all individual clip files are stored.

**How the Code Works:**
1.  **Path Management:** It standardizes file naming, ensuring that every clip ID (like 1, 2, 3) is correctly translated into a specific file name (e.g., "1.json") and located precisely within the project structure.
2.  **Current Clip Tracking:** A key function is remembering the currently active clip. It uses a small, dedicated configuration file (`current.cfg`) to persistently store the name of the last used clip file, allowing the application to resume seamlessly.
    *   `setCurrent` writes the new active clip name to this tracking file.
    *   `loadCurrent` reads the active clip name when the program starts.
3.  **Data Persistence (JSON):** The clip settings themselves are stored using the standard JSON format, which is ideal for structured data.
    *   The `serialize` function saves (writes) complex settings from the program into the designated clip file on disk.
    *   The `hydrate` function loads (reads) and parses those settings back into the program for immediate use.

In summary, this class provides robust mechanisms for managing where clip data lives, ensuring the correct clip is always loaded, and saving all configuration details accurately.

sha: 7bf4a1008bfdbe59bfa669502ee5602e6651b5b21e92a57ffdb0e14b0a9cc275 
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
