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