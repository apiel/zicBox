/** Description:
This code defines a core structure called the `Workspace` manager. Its fundamental job is to organize and handle different user environments, configuration sets, or projects within the computer's file system. Think of it as managing different profiles or save slots for an application.

### How the Workspace Manager Works

**1. Structure and Initialization:**
The manager designates a specific main folder on the host system where all individual workspaces are stored as subdirectories.
*   **Setup (`init`):** When the application starts, this function checks for the main storage folder. It also reads a special configuration file to determine which workspace was active during the previous session, ensuring the application resumes where the user left off. If the last used workspace doesn't exist, it uses a default setup.

**2. Managing Environments:**
The manager provides controls for manipulating these environments:
*   **Creation (`create`):** It can quickly set up a new, empty workspace by creating a dedicated folder for it.
*   **Deletion (`remove`):** It allows the user to completely erase an environment and all its contents from the file system.
*   **Switching (`setCurrent`):** This is the key function for activating a profile. It first ensures the desired workspace exists, and then it immediately updates the special configuration file, making the selected profile the default for future application launches.

**3. Status and Tracking:**
The manager maintains simple internal records:
*   It tracks the name of the currently active workspace (e.g., "GameSettingsProfileA").
*   It provides a function (`getCurrentPath`) to quickly return the full computer address (path) where the files for the active workspace are located.
*   A counter tracks internal changes, allowing other parts of the application to know when a new workspace has been added and needs updating.

sha: 8b8f18827fd2c842f47d99dc63544ba0daca6ba10758a2e81b8a925f6c507b48 
*/
#pragma once

#include "host/constants.h"

#include <filesystem>
#include <fstream>
#include <string>

class Workspace {
public:
    int refreshState = 0;

    std::string folder = WORKSPACE_FOLDER;
    std::string currentCfg = folder + "/current.cfg";
    std::string current = "default";

    std::string getCurrentPath()
    {
        return folder + "/" + current;
    }

    void setCurrent(std::string workspaceName)
    {
        create(workspaceName);
        std::filesystem::create_directories(folder);
        std::ofstream file(currentCfg);
        file << workspaceName;
        file.close();
    }

    void remove(std::string workspaceName)
    {
        std::filesystem::remove_all(folder + "/" + workspaceName);
    }

    void create(std::string workspaceName)
    {
        if (!workspaceName.empty()) {
            std::filesystem::create_directories(folder + "/" + workspaceName);
            refreshState++;
        }
    }

    void init()
    {
        std::filesystem::create_directories(folder);
        currentCfg = folder + "/current.cfg";
        if (std::filesystem::exists(currentCfg)) {
            std::ifstream file(currentCfg);
            std::string line;
            std::getline(file, line);
            file.close();
            if (line.length() > 0) {
                current = line;
            }
        }
        std::filesystem::create_directories(folder + "/" + current);
    }
};