#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>

#include "audioPlugin.h"
#include "helpers/trim.h"
#include "host/constants.h"
#include "log.h"
#include "mapping.h"

/*md
## SerializeTrack

SerializeTrack plugin is used to serialize track on disk. It will scan all the plugins on the given track and save them on disk.

*/
class SerializeTrack : public Mapping {
protected:
    std::mutex m;

    std::string workspaceFolder = WORKSPACE_FOLDER;
    std::string currentCfg = workspaceFolder + "/current.cfg";
    std::string currentWorkspaceName = "default";
    std::string serializePath = workspaceFolder + "/" + currentWorkspaceName + "/track";
    std::string serializeFilename = serializePath + ".json";
    std::string filename = "track";

    bool initialized = false;
    bool saveBeforeChangingVariation = false;
    int refreshState = 0;

    void saveCurrentWorkspaceName(std::string workspaceName)
    {
        createWorkspace(workspaceName);
        std::filesystem::create_directories(workspaceFolder);
        std::ofstream file(currentCfg);
        file << workspaceName;
        file.close();
    }

    std::string getVariationFilepath(int16_t id)
    {
        return serializePath + "/" + std::to_string(id) + ".json";
    }

    void saveVariation(int16_t id)
    {
        serialize();
        std::filesystem::create_directories(serializePath);
        std::filesystem::copy(serializeFilename, getVariationFilepath(id), std::filesystem::copy_options::overwrite_existing);
    }

    void loadVariation(int16_t id)
    {
        // printf("load variation %d\n", id);
        if (std::filesystem::exists(getVariationFilepath(id))) {
            std::filesystem::copy(getVariationFilepath(id), serializeFilename, std::filesystem::copy_options::overwrite_existing);
            hydrate();
        }
    }

    void setVariation(float value)
    {
        // logDebug("set variation %f", value);
        m.lock();
        int16_t currentVariation = variation.get();
        variation.setFloat((int16_t)value);
        if (currentVariation != variation.get() && saveBeforeChangingVariation) {
            saveVariation(currentVariation);
        }
        loadVariation((int16_t)variation.get());
        m.unlock();
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
        currentCfg = workspaceFolder + "/current.cfg";
        if (std::filesystem::exists(currentCfg)) {
            std::ifstream file(currentCfg);
            std::string line;
            std::getline(file, line);
            file.close();
            if (line.length() > 0) {
                currentWorkspaceName = line;
            }
        }
        // logDebug("------------------------------> current workspace: %s [%s][%s]", currentWorkspaceName.c_str(), workspaceFolder.c_str(), currentCfg.c_str());
        std::filesystem::create_directories(workspaceFolder + "/" + currentWorkspaceName);
        serializePath = workspaceFolder + "/" + currentWorkspaceName + "/" + filename;
        serializeFilename = serializePath + ".json";
    }

public:
    /*md **Values:** */
    /*md - `VARIATION` switch between different track serialization variations (clip). */
    Val& variation = val(0.0f, "VARIATION", { "Variation", .max = 1000.0f }, [&](auto p) { setVariation(p.value); });

    SerializeTrack(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        //md **Config**:
        auto& json = config.json;

        //md - `"maxVariation": 12` to set max variation. By default it is `12`.
        if (json.contains("maxVariation")) {
            variation.props().max = json["maxVariation"].get<int>();
        }

        //md - `"saveBeforeChangingVariation": true` toggle to enable variation edit mode. If set to false variation will be read only. If set to true, every changes will be save before to switch to the next variation. Default is false`.
        saveBeforeChangingVariation = json.value("saveBeforeChangingVariation", saveBeforeChangingVariation);

        //md - `"filename": "track"` to set filename. By default it is `track`.
        filename = json.value("filename", filename);

        //md - `"workspaceFolder": "data/workspaces"` to set workspace folder.
        workspaceFolder = json.value("workspaceFolder", workspaceFolder);

        initFilepath();
    }

    void sample(float* buf)
    {
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
            if (saveBeforeChangingVariation) {
                serialize(); // save current workspace before to switch
            }
            initFilepath(); // set new workspace
            hydrate(); // load new workspace
            m.unlock();
        } else if (event == AudioEventType::RELOAD_VARIATION) {
            m.lock();
            loadVariation(variation.get());
            m.unlock();
        } else if (event == AudioEventType::SAVE_VARIATION) {
            m.lock();
            saveVariation(variation.get());
            m.unlock();
        }
    }

    void savePreset()
    {
        nlohmann::json json;
        for (AudioPlugin* plugin : props.audioPluginHandler->plugins) {
            if ((track == -1 || track == plugin->track) && plugin->preset) {
                plugin->serializeJson(json);
            }
        }
        logDebug("preset json: %s", json.dump(4).c_str());
        // FILE* jsonFile = fopen((serializeFilename).c_str(), "w");
        // fprintf(jsonFile, "%s", json.dump(4).c_str());
        // fclose(jsonFile);
    }

    void serialize()
    {
        nlohmann::json json;
        for (AudioPlugin* plugin : props.audioPluginHandler->plugins) {
            if ((track == -1 || track == plugin->track) && plugin->serializable) {
                plugin->serializeJson(json[plugin->name]);
            }
        }
        FILE* jsonFile = fopen((serializeFilename).c_str(), "w");
        fprintf(jsonFile, "%s", json.dump(4).c_str());
        fclose(jsonFile);
    }

    void hydrate()
    {
        std::string filepath = serializeFilename;
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

    void serializeJson(nlohmann::json& json) override
    {
        json["VARIATION"] = variation.get();
    }

    std::vector<int> variationExists = std::vector<int>(1000, -1);
    std::string dataStr;
    DataFn dataFunctions[14] = {
        { "SERIALIZE", [this](void* userdata) {
             data(0, userdata);
             m.lock();
             serialize();
             m.unlock();
             return (void*)NULL;
         } },
        { "HYDRATE", [this](void* userdata) {
             data(0, userdata);
             m.lock();
             hydrate();
             m.unlock();
             return (void*)NULL;
         } },
        { "VARIATION_EXISTS", [this](void* userdata) {
             if (userdata) {
                 int id = *(int16_t*)userdata;
                 if (variationExists[id] == -1) {
                     bool fileExists = std::filesystem::exists(getVariationFilepath(id));
                     variationExists[id] = fileExists ? 1 : 0;
                 }
                 //  return (void*)&variationExists[id];
                 return variationExists[id] == 1 ? (void*)true : (void*)NULL;
             }
             return (void*)NULL;
         } },
        { "GET_VARIATION_PATH", [this](void* userdata) {
             if (userdata) {
                 int id = *(int16_t*)userdata;
                 dataStr = getVariationFilepath(id);
                 return (void*)&dataStr;
             }
             return (void*)NULL;
         } },
        { "SAVE_VARIATION", [this](void* userdata) {
             if (userdata) {
                 int id = *(int16_t*)userdata;
                 m.lock();
                 saveVariation(id);
                 m.unlock();
                 variationExists[id] = 1;
                 variation.setFloat(id);
             }
             return (void*)NULL;
         } },
        { "LOAD_VARIATION", [this](void* userdata) {
             if (userdata) {
                 nextVariationToPlay = -1;
                 int id = *(int16_t*)userdata;
                 // m.lock();
                 // loadVariation(id);
                 // m.unlock();
                 setVariation(id);
             }
             return (void*)NULL;
         } },
        { "LOAD_VARIATION_NEXT", [this](void* userdata) {
             if (userdata) {
                 nextVariationToPlay = *(int16_t*)userdata;
             }
             return (void*)&nextVariationToPlay;
         } },
        { "DELETE_VARIATION", [this](void* userdata) {
             if (userdata) {
                 int id = *(int16_t*)userdata;
                 std::filesystem::remove(getVariationFilepath(id));
                 variationExists[id] = 0;
             }
             return (void*)NULL;
         } },
        { "CURRENT_WORKSPACE", [this](void* userdata) {
             return (void*)&currentWorkspaceName;
         } },
        { "DELETE_WORKSPACE", [this](void* userdata) {
             if (userdata) {
                 std::string workspaceName = *(std::string*)userdata;
                 logDebug("Delete workspace %s", workspaceName.c_str());
                 deleteWorkspace(workspaceName);
             }
             return (void*)NULL;
         } },
        { "CREATE_WORKSPACE", [this](void* userdata) {
             if (userdata) {
                 std::string workspaceName = *(std::string*)userdata;
                 logDebug("Create workspace %s", workspaceName.c_str());
                 createWorkspace(workspaceName);
             }
             return (void*)&refreshState;
         } },
        { "LOAD_WORKSPACE", [this](void* userdata) {
             if (userdata) {
                 std::string workspaceName = *(std::string*)userdata;
                 if (workspaceName != currentWorkspaceName) {
                     saveCurrentWorkspaceName(workspaceName);
                     props.audioPluginHandler->sendEvent(AudioEventType::RELOAD_WORKSPACE);
                 }
             }
             return (void*)NULL;
         } },
        { "WORKSPACE_FOLDER", [this](void* userdata) {
             return (void*)&workspaceFolder;
         } },
        { "SAVE_PRESET", [this](void* userdata) {
            //  if (userdata) {
                //  int id = *(int16_t*)userdata;
                 m.lock();
                 savePreset();
                 m.unlock();
            //  }
             return (void*)NULL;
         } },
    };

    DEFINE_GETDATAID_AND_DATA
};
