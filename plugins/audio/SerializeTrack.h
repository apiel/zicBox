#pragma once

#include <iostream>
#include <mutex>
#include <sstream>

#include "audioPlugin.h"
#include "helpers/trim.h"
#include "host/constants.h"
#include "log.h"
#include "mapping.h"
#include "plugins/audio/utils/Clip.h"

/*md
## SerializeTrack

SerializeTrack plugin is used to serialize track on disk. It will scan all the plugins on the given track and save them on disk.

*/
class SerializeTrack : public Mapping {
protected:
    std::mutex m;

    Clip clip;

    bool initialized = false;
    bool saveBeforeChangingClip = false;

    void saveClip(int16_t id)
    {
        serialize();
        clip.save(id);
    }

    void loadClip(int16_t id)
    {
        if (clip.load(id)) {
            hydrate();
        }
    }

    void setClip(float value)
    {
        // logDebug("set clip %f", value);
        m.lock();
        int16_t currentClip = clipVal.get();
        clipVal.setFloat((int16_t)value);
        if (currentClip != clipVal.get() && saveBeforeChangingClip) {
            saveClip(currentClip);
        }
        loadClip((int16_t)clipVal.get());
        m.unlock();
    }

public:
    /*md **Values:** */
    /*md - `CLIP` switch between different track serialization clips (clip). */
    Val& clipVal = val(0.0f, "CLIP", { "Clip", .max = 1000.0f }, [&](auto p) { setClip(p.value); });

    SerializeTrack(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        //md **Config**:
        auto& json = config.json;

        //md - `"maxClip": 12` to set max clipVal. By default it is `12`.
        if (json.contains("maxClip")) {
            clipVal.props().max = json["maxClip"].get<int>();
        }

        //md - `"saveBeforeChangingClip": true` toggle to enable clip edit mode. If set to false clip will be read only. If set to true, every changes will be save before to switch to the next clipVal. Default is false`.
        saveBeforeChangingClip = json.value("saveBeforeChangingClip", saveBeforeChangingClip);

        clip.config(json);
    }

    void sample(float* buf)
    {
    }

    int nextClipToPlay = -1;
    void onEvent(AudioEventType event, bool isPlaying) override
    {
        if (event == AudioEventType::SEQ_LOOP) {
            if (nextClipToPlay != -1) {
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
            clip.init();
            hydrate(); // load new workspace
            m.unlock();
        } else if (event == AudioEventType::RELOAD_CLIP) {
            m.lock();
            loadClip(clipVal.get());
            m.unlock();
        } else if (event == AudioEventType::SAVE_CLIP) {
            m.lock();
            saveClip(clipVal.get());
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
        FILE* jsonFile = fopen((clip.serializeFilename).c_str(), "w");
        fprintf(jsonFile, "%s", json.dump(4).c_str());
        fclose(jsonFile);
    }

    void hydrate()
    {
        std::string filepath = clip.serializeFilename;
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
            clipVal.setFloat(json["CLIP"]);
        }
    }

    void serializeJson(nlohmann::json& json) override
    {
        json["CLIP"] = clipVal.get();
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
                     bool fileExists = std::filesystem::exists(clip.getFilepath(id));
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
                 dataStr = clip.getFilepath(id);
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
                 clipVal.setFloat(id);
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
                 std::filesystem::remove(clip.getFilepath(id));
                 clipExists[id] = 0;
             }
             return (void*)NULL;
         } },
        { "CURRENT_WORKSPACE", [this](void* userdata) {
             return (void*)&clip.workspace.current;
         } },
        { "DELETE_WORKSPACE", [this](void* userdata) {
             if (userdata) {
                 std::string workspaceName = *(std::string*)userdata;
                 logDebug("Delete workspace %s", workspaceName.c_str());
                 clip.workspace.remove(workspaceName);
             }
             return (void*)NULL;
         } },
        { "CREATE_WORKSPACE", [this](void* userdata) {
             if (userdata) {
                 std::string workspaceName = *(std::string*)userdata;
                 logDebug("Create workspace %s", workspaceName.c_str());
                 clip.workspace.create(workspaceName);
             }
             return (void*)&clip.workspace.refreshState;
         } },
        { "LOAD_WORKSPACE", [this](void* userdata) {
             if (userdata) {
                 std::string workspaceName = *(std::string*)userdata;
                 if (workspaceName != clip.workspace.current) {
                     clip.workspace.saveCurrent(workspaceName);
                     props.audioPluginHandler->sendEvent(AudioEventType::RELOAD_WORKSPACE);
                 }
             }
             return (void*)NULL;
         } },
        { "WORKSPACE_FOLDER", [this](void* userdata) {
             return (void*)&clip.workspace.folder;
         } },
    };

    DEFINE_GETDATAID_AND_DATA
};
