#pragma once

#include <nlohmann/json.hpp>
#include <string>
// #include <vector>

#include "log.h"
#include "plugins/audio/utils/Workspace.h"

class Clip {
protected:

public:
    Workspace workspace;

    std::string serializePath = workspace.getCurrentPath() + "/track";
    std::string serializeFilename = serializePath + ".json";
    std::string filename = "track";

    std::string getFilepath(int16_t id)
    {
        return serializePath + "/" + std::to_string(id) + ".json";
    }

    void save(int16_t id)
    {
        std::filesystem::create_directories(serializePath);
        std::filesystem::copy(serializeFilename, getFilepath(id), std::filesystem::copy_options::overwrite_existing);
    }

    bool load(int16_t id)
    {
        // printf("load clip %d\n", id);
        if (std::filesystem::exists(getFilepath(id))) {
            std::filesystem::copy(getFilepath(id), serializeFilename, std::filesystem::copy_options::overwrite_existing);
            return true;
        }
        return false;
    }

    void config(nlohmann::json& json)
    {
        workspace.folder = json.value("workspaceFolder", workspace.folder);
        workspace.init();

        filename = json.value("filename", filename);

        init();
    }

    void init()
    {
        workspace.init();
        serializePath = workspace.getCurrentPath() + "/" + filename;
        serializeFilename = serializePath + ".json";
    }

    void serialize(nlohmann::json& json)
    {
        std::ofstream file(serializeFilename);
        if (file.is_open()) {
            logDebug("Saving clip: %s", serializeFilename.c_str());
            file << json.dump(4);
            file.close();
        }
    }

    nlohmann::json hydrate()
    {
        std::ifstream file(serializeFilename);
        if (file.is_open()) {
            logDebug("Loading clip: %s", serializeFilename.c_str());
            nlohmann::json json;
            file >> json;
            file.close();
            return json;
        } else {
            logWarn("Unable to open clip file: %s", serializeFilename.c_str());
            return nlohmann::json();
        }
    }
};
