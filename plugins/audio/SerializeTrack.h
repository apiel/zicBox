#pragma once

#include <iostream>
#include <mutex>
#include <sstream>

#include "audioPlugin.h"
#include "helpers/trim.h"
#include "host/constants.h"
#include "log.h"
#include "mapping.h"
#include "plugins/audio/utils/Workspace.h"

/*md
## SerializeTrack

SerializeTrack plugin is used to serialize track on disk. It will scan all the plugins on the given track and save them on disk.

*/
class SerializeTrack : public Mapping {
protected:
    std::mutex m;

    Workspace workspace;

    std::string serializePath = workspace.folder + "/" + workspace.current + "/track";
    std::string serializeFilename = serializePath + ".json";
    std::string filename = "track";

    bool initialized = false;
    bool saveBeforeChangingClip = false;

    std::string getClipFilepath(int16_t id)
    {
        return serializePath + "/" + std::to_string(id) + ".json";
    }

    void saveClip(int16_t id)
    {
        serialize();
        std::filesystem::create_directories(serializePath);
        std::filesystem::copy(serializeFilename, getClipFilepath(id), std::filesystem::copy_options::overwrite_existing);
    }

    void loadClip(int16_t id)
    {
        // printf("load clip %d\n", id);
        if (std::filesystem::exists(getClipFilepath(id))) {
            std::filesystem::copy(getClipFilepath(id), serializeFilename, std::filesystem::copy_options::overwrite_existing);
            hydrate();
        }
    }

    void setClip(float value)
    {
        // logDebug("set clip %f", value);
        m.lock();
        int16_t currentClip = clip.get();
        clip.setFloat((int16_t)value);
        if (currentClip != clip.get() && saveBeforeChangingClip) {
            saveClip(currentClip);
        }
        loadClip((int16_t)clip.get());
        m.unlock();
    }

    void initFilepath()
    {
        workspace.init();
        serializePath = workspace.folder + "/" + workspace.current + "/" + filename;
        serializeFilename = serializePath + ".json";
    }

public:
    /*md **Values:** */
    /*md - `CLIP` switch between different track serialization clips (clip). */
    Val& clip = val(0.0f, "CLIP", { "Clip", .max = 1000.0f }, [&](auto p) { setClip(p.value); });

    SerializeTrack(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        //md **Config**:
        auto& json = config.json;

        //md - `"maxClip": 12` to set max clip. By default it is `12`.
        if (json.contains("maxClip")) {
            clip.props().max = json["maxClip"].get<int>();
        }

        //md - `"saveBeforeChangingClip": true` toggle to enable clip edit mode. If set to false clip will be read only. If set to true, every changes will be save before to switch to the next clip. Default is false`.
        saveBeforeChangingClip = json.value("saveBeforeChangingClip", saveBeforeChangingClip);

        //md - `"filename": "track"` to set filename. By default it is `track`.
        filename = json.value("filename", filename);

        //md - `"workspaceFolder": "data/workspaces"` to set workspace folder.
        workspace.folder = json.value("workspaceFolder", workspace.folder);

        initFilepath();
    }

    void sample(float* buf)
    {
    }

    int nextClipToPlay = -1;
    void onEvent(AudioEventType event, bool isPlaying) override
    {
        if (event == AudioEventType::SEQ_LOOP) {
            if (nextClipToPlay != -1) {
                // m.lock();
                // loadClip(nextClipToPlay);
                // m.unlock();
                setClip(nextClipToPlay);
                nextClipToPlay = -1;
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
            if (saveBeforeChangingClip) {
                serialize(); // save current workspace before to switch
            }
            initFilepath(); // set new workspace
            hydrate(); // load new workspace
            m.unlock();
        } else if (event == AudioEventType::RELOAD_CLIP) {
            m.lock();
            loadClip(clip.get());
            m.unlock();
        } else if (event == AudioEventType::SAVE_CLIP) {
            m.lock();
            saveClip(clip.get());
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
        if (!initialized && json.contains("CLIP")) {
            clip.setFloat(json["CLIP"]);
        }
    }

    void serializeJson(nlohmann::json& json) override
    {
        json["CLIP"] = clip.get();
    }

    std::vector<int> clipExists = std::vector<int>(1000, -1);
    std::string dataStr;
    DataFn dataFunctions[13] = {
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
        { "CLIP_EXISTS", [this](void* userdata) {
             if (userdata) {
                 int id = *(int16_t*)userdata;
                 if (clipExists[id] == -1) {
                     bool fileExists = std::filesystem::exists(getClipFilepath(id));
                     clipExists[id] = fileExists ? 1 : 0;
                 }
                 //  return (void*)&clipExists[id];
                 return clipExists[id] == 1 ? (void*)true : (void*)NULL;
             }
             return (void*)NULL;
         } },
        { "GET_CLIP_PATH", [this](void* userdata) {
             if (userdata) {
                 int id = *(int16_t*)userdata;
                 dataStr = getClipFilepath(id);
                 return (void*)&dataStr;
             }
             return (void*)NULL;
         } },
        { "SAVE_CLIP", [this](void* userdata) {
             if (userdata) {
                 int id = *(int16_t*)userdata;
                 m.lock();
                 saveClip(id);
                 m.unlock();
                 clipExists[id] = 1;
                 clip.setFloat(id);
             }
             return (void*)NULL;
         } },
        { "LOAD_CLIP", [this](void* userdata) {
             if (userdata) {
                 nextClipToPlay = -1;
                 int id = *(int16_t*)userdata;
                 // m.lock();
                 // loadClip(id);
                 // m.unlock();
                 setClip(id);
             }
             return (void*)NULL;
         } },
        { "LOAD_CLIP_NEXT", [this](void* userdata) {
             if (userdata) {
                 nextClipToPlay = *(int16_t*)userdata;
             }
             return (void*)&nextClipToPlay;
         } },
        { "DELETE_CLIP", [this](void* userdata) {
             if (userdata) {
                 int id = *(int16_t*)userdata;
                 std::filesystem::remove(getClipFilepath(id));
                 clipExists[id] = 0;
             }
             return (void*)NULL;
         } },
        { "CURRENT_WORKSPACE", [this](void* userdata) {
             return (void*)&workspace.current;
         } },
        { "DELETE_WORKSPACE", [this](void* userdata) {
             if (userdata) {
                 std::string workspaceName = *(std::string*)userdata;
                 logDebug("Delete workspace %s", workspaceName.c_str());
                 workspace.remove(workspaceName);
             }
             return (void*)NULL;
         } },
        { "CREATE_WORKSPACE", [this](void* userdata) {
             if (userdata) {
                 std::string workspaceName = *(std::string*)userdata;
                 logDebug("Create workspace %s", workspaceName.c_str());
                 workspace.create(workspaceName);
             }
             return (void*)&workspace.refreshState;
         } },
        { "LOAD_WORKSPACE", [this](void* userdata) {
             if (userdata) {
                 std::string workspaceName = *(std::string*)userdata;
                 if (workspaceName != workspace.current) {
                     workspace.saveCurrent(workspaceName);
                     props.audioPluginHandler->sendEvent(AudioEventType::RELOAD_WORKSPACE);
                 }
             }
             return (void*)NULL;
         } },
        { "WORKSPACE_FOLDER", [this](void* userdata) {
             return (void*)&workspace.folder;
         } },
    };

    DEFINE_GETDATAID_AND_DATA
};
