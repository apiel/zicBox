/** Description:
## SerializeTrack Component Analysis

This component, named `SerializeTrack`, functions as a specialized configuration management tool within an audio processing environment. It does not manipulate audio signals directly but rather focuses entirely on saving and loading the settings of all other audio plugins connected to a specific track.

### 1. Core Purpose and Mechanism

The primary role of `SerializeTrack` is to capture the entire state of an audio track and save it to persistent storage, effectively creating a "snapshot" or a "Clip."

*   **Saving (Serialization):** It scans all relevant plugins on the track and converts their current operational settings and parameters into a structured data format (like a configuration file).
*   **Loading (Hydration):** It reads a previously saved configuration file and applies those settings back to the corresponding plugins, instantly restoring the track to a saved state.

### 2. Key Features (Clips and Control)

The component manages these snapshots using a concept called **Clips**.

*   **Clip Selection:** A central control parameter (`CLIP`) allows the user or host application to switch between different numbered Clips (presets). When this parameter is changed, the component automatically loads the settings for that new Clip.
*   **Save Mode:** A setting determines if the component should automatically save the current Clip's changes *before* switching to a new one, ensuring work is not lost during rapid preset changes.
*   **Workspaces:** It supports the management of different "Workspaces," which are essentially separate folders or collections used to organize large sets of Clips.

### 3. Execution and Safety

Since saving and loading files are complex operations, the component ensures data integrity:

*   **Thread Safety:** All critical save and load operations are protected by an internal locking mechanism (a "mutex"). This prevents data corruption if two different parts of the host application try to access or modify the file state simultaneously.
*   **Event Handling:** It listens for system-wide events. For instance, it triggers a save during an `AUTOSAVE` event or handles the loading of new settings when a `RELOAD_WORKSPACE` event occurs. It can also manage scheduled Clip changes based on sequencing loop events.

### 4. External Access

The component offers several structured commands (data functions) that allow other parts of the application to interrogate or control the saving process, such as manually checking if a specific Clip number exists, retrieving a Clip’s file path, or explicitly triggering a save or load operation.

sha: bcac1e338ba4016c92e7d6c830ed120597f97693af274231bf3576a2a6dec613 
*/
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
        clip.setCurrent(id);
        serialize();
    }

    void loadClip(int16_t id)
    {
        clip.setCurrent(id);
        hydrate();
    }

    void setClip(float value)
    {
        // logDebug("set clip %f", value);
        m.lock();
        int16_t currentClip = clipVal.get();
        clipVal.setFloat((int16_t)value);
        if (currentClip != clipVal.get() && saveBeforeChangingClip) {
            // saveClip(currentClip);
            serialize(false);
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

        //md - `"saveBeforeChangingClip": true` toggle to enable clip edit mode. If set to false clip will be read only. If set to true, every changes will be save before to switch to the next clipVal. Default is false`.
        saveBeforeChangingClip = json.value("saveBeforeChangingClip", saveBeforeChangingClip);

        clip.config(json);
        clipVal.props().max = clip.getMaxClips() - 1;
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
            clip.init();
            hydrate(true); // load new workspace
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

    void serialize(bool toFile = true)
    {
        nlohmann::json json;
        for (AudioPlugin* plugin : props.audioPluginHandler->plugins) {
            if ((track == -1 || track == plugin->track) && plugin->serializable) {
                plugin->serializeJson(json[plugin->name]);
            }
        }
        clip.serialize(json, toFile);
    }

    void hydrate(bool reload = false)
    {
        try {
            nlohmann::json json = clip.hydrate(reload);

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
            std::string errorMessage = "Error hydrating clip " + clip.current + ": " + std::string(e.what());
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
        // TODO
        // TODO instead to have all those workspace action, let just return a pointer to workspace
        // TODO
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
                     clip.workspace.setCurrent(workspaceName);
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
