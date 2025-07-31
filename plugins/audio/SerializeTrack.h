#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>

#include "audioPlugin.h"
#include "helpers/trim.h"
#include "log.h"
#include "mapping.h"

/*md
## SerializeTrack

SerializeTrack plugin is used to serialize track on disk. It will scan all the plugins on the given track and save them on disk.

*/
class SerializeTrack : public Mapping {
protected:
    std::mutex m;

    std::string workspaceFolder = "data/workspaces";
    std::string filename = "track";
    std::string currentWorkspaceName = "default";

    std::string filepath = "data/workspaces/default/track.json";
    std::string variationFolder = "data/workspaces/default/track";
    bool initialized = false;

    bool saveBeforeChangingVariation = false;

    int refreshState = 0;
    void saveCurrentWorkspaceName(std::string workspaceName)
    {
        std::filesystem::create_directories(workspaceFolder);
        std::string currentWorkspaceFile = workspaceFolder + "/current.cfg";
        std::ofstream file(currentWorkspaceFile);
        file << workspaceName;
        file.close();
    }

    void deleteWorkspace(std::string workspaceName)
    {
        std::filesystem::remove_all(workspaceFolder + "/" + workspaceName);
    }

    void createWorkspace(std::string workspaceName)
    {
        if (!workspaceName.empty()) {
            std::filesystem::create_directories(workspaceFolder + "/" + workspaceName);
            refreshState++;
        }
    }

    void initFilepath()
    {
        std::filesystem::create_directories(workspaceFolder);
        std::string currentWorkspaceFile = workspaceFolder + "/current.cfg";
        if (std::filesystem::exists(currentWorkspaceFile)) {
            std::ifstream file(currentWorkspaceFile);
            std::string line;
            std::getline(file, line);
            file.close();
            if (line.length() > 0) {
                currentWorkspaceName = line;
            }
        }
        std::string currentWorkspaceFolder = workspaceFolder + "/" + currentWorkspaceName;
        std::filesystem::create_directories(currentWorkspaceFolder);
        filepath = currentWorkspaceFolder + "/" + filename + ".json";
        variationFolder = currentWorkspaceFolder + "/" + filename;
    }

public:
    /*md **Values:** */
    /*md - `VARIATION` switch between different track serialization variations (clip). */
    Val& variation = val(0.0f, "VARIATION", { "Variation", .max = 12.0f }, [&](auto p) { setVariation(p.value); });

    SerializeTrack(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initFilepath();

        //md **Config**:
        auto& json = config.json;
        //md - `"filename": "track"` to set filename. By default it is `track`.
        if (json.contains("filename")) {
            filename = json["filename"].get<std::string>();
            initFilepath();
        }

        //md - `"workspaceFolder": "workspaces"` to set workspace folder. By default it is `data/workspaces`.
        if (json.contains("workspaceFolder")) {
            workspaceFolder = json["workspaceFolder"].get<std::string>();
            initFilepath();
        }

        //md - `"maxVariation": 12` to set max variation. By default it is `12`.
        if (json.contains("maxVariation")) {
            variation.props().max = json["maxVariation"].get<int>();
        }

        //md - `"saveBeforeChangingVariation": true` toggle to enable variation edit mode. If set to false variation will be read only. If set to true, every changes will be save before to switch to the next variation. Default is false`.
        saveBeforeChangingVariation = json.value("saveBeforeChangingVariation", saveBeforeChangingVariation);
    }

    void sample(float* buf)
    {
    }

    std::string getVariationFilepath(int16_t id)
    {
        return variationFolder + "/" + std::to_string(id) + ".json";
    }

    void saveVariation(int16_t id)
    {
        std::filesystem::create_directories(variationFolder);
        std::filesystem::copy(filepath, getVariationFilepath(id), std::filesystem::copy_options::overwrite_existing);
    }

    void loadVariation(int16_t id)
    {
        // printf("load variation %d\n", id);
        if (std::filesystem::exists(getVariationFilepath(id))) {
            std::filesystem::copy(getVariationFilepath(id), filepath, std::filesystem::copy_options::overwrite_existing);
            hydrate();
        }
    }

    void setVariation(float value)
    {
        logDebug("set variation %f\n", value);
        m.lock();
        int16_t currentVariation = variation.get();
        variation.setFloat((int16_t)value);
        if (currentVariation != variation.get() && saveBeforeChangingVariation) {
            saveVariation(currentVariation);
        }
        loadVariation((int16_t)variation.get());
        m.unlock();
    }

    int nextVariationToPlay = -1;
    void onEvent(AudioEventType event, bool isPlaying) override
    {
        if (event == AudioEventType::SEQ_LOOP) {
            if (nextVariationToPlay != -1) {
                // m.lock();
                // loadVariation(nextVariationToPlay);
                // m.unlock();
                setVariation(nextVariationToPlay);
                nextVariationToPlay = -1;
            }
        } else if (event == AudioEventType::AUTOSAVE) {
            if (!initialized) {
                m.lock();
                hydrate();
                m.unlock();
                initialized = true;
            } else {
                m.lock();
                serialize();
                m.unlock();
            }
        } else if (event == AudioEventType::RELOAD_WORKSPACE) {
            m.lock();
            serialize();
            initFilepath();
            hydrate();
            m.unlock();
        }
    }

    void serialize()
    {
        nlohmann::json json;
        for (AudioPlugin* plugin : props.audioPluginHandler->plugins) {
            if ((track == -1 || track == plugin->track) && plugin->serializable) {
                plugin->serializeJson(json[plugin->name]);
            }
        }
        FILE* jsonFile = fopen((filepath).c_str(), "w");
        fprintf(jsonFile, "%s", json.dump(4).c_str());
        fclose(jsonFile);
    }

    void hydrate()
    {
        std::ifstream file(filepath);
        if (!file) {
            logError("Hydration file not found: " + filepath);
            return;
        }

        std::ostringstream ss;
        ss << file.rdbuf(); // Read entire buffer into stream
        std::string buffer = ss.str();
        // logWarn(buffer);

        try {
            nlohmann::json json;
            json = nlohmann::json::parse(buffer);

            // loop through keys
            for (auto it = json.begin(); it != json.end(); ++it) {
                AudioPlugin* plugin = props.audioPluginHandler->getPluginPtr(it.key(), track);
                if (plugin) {
                    if (plugin->serializable) {
                        logDebug("Hydrating plugin: %s on track %d", plugin->name.c_str(), plugin->track);
                        plugin->hydrateJson(it.value());
                    } else {
                        logWarn("Cannot hydrate plugin: %s (not serializable)\n", plugin->name.c_str());
                    }
                } else {
                    logWarn("Cannot hydrate plugin: %s (not found)\n", it.key().c_str());
                }
            }
        } catch (const std::exception& e) {
            std::string errorMessage = "Error hydrating file " + filepath + ".json : " + std::string(e.what());
            logError(errorMessage);
        }
    }

    void hydrateJson(nlohmann::json& json) override
    {
        // Do not hydrate this plugin, else it would make a loop

        if (!initialized && json.contains("VARIATION")) {
            variation.setFloat(json["VARIATION"]);
        }
    }

    enum DATA_ID {
        SET_FILENAME,
        SERIALIZE,
        HYDRATE,
        GET_VARIATION,
        GET_VARIATION_PATH,
        SAVE_VARIATION,
        LOAD_VARIATION,
        LOAD_VARIATION_NEXT,
        DELETE_VARIATION,
        CREATE_WORKSPACE,
        LOAD_WORKSPACE,
        CURRENT_WORKSPACE,
        DELETE_WORKSPACE
    };

    /*md **Data ID**: */
    uint8_t getDataId(std::string name) override
    {
        /*md - `SET_FILENAME` set filename */
        if (name == "SET_FILENAME")
            return DATA_ID::SET_FILENAME;
        /*md - `SERIALIZE` serialize */
        if (name == "SERIALIZE")
            return DATA_ID::SERIALIZE;
        /*md - `HYDRATE` hydrate */
        if (name == "HYDRATE")
            return DATA_ID::HYDRATE;
        /*md - `GET_VARIATION` get variation */
        if (name == "GET_VARIATION")
            return DATA_ID::GET_VARIATION;
        /*md - `GET_VARIATION_PATH` get variation path */
        if (name == "GET_VARIATION_PATH")
            return DATA_ID::GET_VARIATION_PATH;
        /*md - `SAVE_VARIATION` save variation */
        if (name == "SAVE_VARIATION")
            return DATA_ID::SAVE_VARIATION;
        /*md - `LOAD_VARIATION` load variation */
        if (name == "LOAD_VARIATION")
            return DATA_ID::LOAD_VARIATION;
        /*md - `LOAD_VARIATION_NEXT` load variation at the next sequencer loop */
        if (name == "LOAD_VARIATION_NEXT")
            return DATA_ID::LOAD_VARIATION_NEXT;
        /*md - `DELETE_VARIATION` delete variation */
        if (name == "DELETE_VARIATION")
            return DATA_ID::DELETE_VARIATION;
        /*md - `CREATE_WORKSPACE` create workspace */
        if (name == "CREATE_WORKSPACE")
            return DATA_ID::CREATE_WORKSPACE;
        /*md - `LOAD_WORKSPACE` load workspace */
        if (name == "LOAD_WORKSPACE")
            return DATA_ID::LOAD_WORKSPACE;
        /*md - `CURRENT_WORKSPACE` get current workspace */
        if (name == "CURRENT_WORKSPACE")
            return DATA_ID::CURRENT_WORKSPACE;
        /*md - `DELETE_WORKSPACE` delete workspace */
        if (name == "DELETE_WORKSPACE")
            return DATA_ID::DELETE_WORKSPACE;
        return atoi(name.c_str());
    }

    std::string dataStr;
    void* data(int id, void* userdata = NULL)
    {
        switch (id) {
        case DATA_ID::SET_FILENAME: {
            if (userdata) {
                filename = (char*)userdata;
                initFilepath();
            }
            return NULL;
        }
        case DATA_ID::SERIALIZE:
            data(0, userdata);
            m.lock();
            serialize();
            m.unlock();
            return NULL;
        case DATA_ID::HYDRATE:
            data(0, userdata);
            m.lock();
            hydrate();
            m.unlock();
            return NULL;
        case DATA_ID::GET_VARIATION: {
            if (userdata) {
                int id = *(int16_t*)userdata;
                bool fileExists = std::filesystem::exists(getVariationFilepath(id));
                return fileExists ? &fileExists : NULL;
            }
            return NULL;
        }
        case DATA_ID::GET_VARIATION_PATH: {
            if (userdata) {
                int id = *(int16_t*)userdata;
                dataStr = getVariationFilepath(id);
                return &dataStr;
            }
            return NULL;
        }
        case DATA_ID::SAVE_VARIATION: {
            if (userdata) {
                int id = *(int16_t*)userdata;
                m.lock();
                saveVariation(id);
                m.unlock();
                variation.setFloat(id);
            }
            return NULL;
        }
        case DATA_ID::LOAD_VARIATION: {
            if (userdata) {
                nextVariationToPlay = -1;
                int id = *(int16_t*)userdata;
                // m.lock();
                // loadVariation(id);
                // m.unlock();
                setVariation(id);
            }
            return NULL;
        }
        case DATA_ID::LOAD_VARIATION_NEXT:
            nextVariationToPlay = *(int16_t*)userdata;
            return &nextVariationToPlay;
        case DATA_ID::DELETE_VARIATION: {
            if (userdata) {
                int id = *(int16_t*)userdata;
                std::filesystem::remove(getVariationFilepath(id));
            }
            return NULL;
        }
        case DATA_ID::CURRENT_WORKSPACE:
            return &currentWorkspaceName;
        case DATA_ID::DELETE_WORKSPACE: {
            if (userdata) {
                std::string workspaceName = *(std::string*)userdata;
                printf("Delete workspace %s\n", workspaceName.c_str());
                deleteWorkspace(workspaceName);
            }
            return NULL;
        }
        case DATA_ID::CREATE_WORKSPACE: {
            if (userdata) {
                std::string workspaceName = *(std::string*)userdata;
                printf("Create workspace %s\n", workspaceName.c_str());
                createWorkspace(workspaceName);
            }
            return &refreshState;
        }
        case DATA_ID::LOAD_WORKSPACE: {
            if (userdata) {
                std::string workspaceName = *(std::string*)userdata;
                printf("Load workspace %s\n", workspaceName.c_str());
                if (workspaceName != currentWorkspaceName) {
                    saveCurrentWorkspaceName(workspaceName);
                    props.audioPluginHandler->sendEvent(AudioEventType::RELOAD_WORKSPACE);
                }
            }
            return NULL;
        }
        }
        return NULL;
    }
};
