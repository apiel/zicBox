#ifndef _SERIALIZE_TRACK_H_
#define _SERIALIZE_TRACK_H_

#include <filesystem>
#include <mutex>

#include "../../helpers/trim.h"
#include "audioPlugin.h"
#include "mapping.h"

/*md
## SerializeTrack

SerializeTrack plugin is used to serialize track on disk. It will scan all the plugins on the given track and save them on disk.

*/
class SerializeTrack : public Mapping {
protected:
    std::mutex m;

    std::string filepath = "serialized/track.cfg";
    std::string variationFolder = "serialized/track";
    bool initialized = false;

    bool saveBeforeChangingVariation = false;

    void setFilepath(std::string newFilepath)
    {
        filepath = newFilepath;
        const char* dot = strrchr(filepath.c_str(), '.');
        const char* slash = strrchr(filepath.c_str(), '/');
        if (dot && slash && dot > slash) {
            variationFolder = filepath.substr(0, dot - filepath.c_str());
        }
    }

public:
    /*md **Values:** */
    /*md - `VARIATION` switch between different track serialization variations (clip). */
    Val& variation = val(0.0f, "VARIATION", { "Variation", .max = 12.0f }, [&](auto p) { setVariation(p.value); });

    SerializeTrack(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
    }

    void sample(float* buf)
    {
    }

    std::string getVariationFilepath(int16_t id)
    {
        return variationFolder + "/" + std::to_string(id) + ".cfg";
    }

    void saveVariation(int16_t id)
    {
        std::filesystem::create_directories(variationFolder);
        std::filesystem::copy(filepath, getVariationFilepath(id), std::filesystem::copy_options::overwrite_existing);
    }

    void loadVariation(int16_t id)
    {
        if (std::filesystem::exists(getVariationFilepath(id))) {
            std::filesystem::copy(getVariationFilepath(id), filepath, std::filesystem::copy_options::overwrite_existing);
            hydrate();
        }
    }

    void setVariation(float value)
    {
        printf("set variation %f\n", value);
        m.lock();
        int16_t currentVariation = variation.get();
        variation.setFloat((int16_t)value);
        if (currentVariation != variation.get()) {
            if (saveBeforeChangingVariation) {
                saveVariation(currentVariation);
            }
        }
        loadVariation((int16_t)variation.get());
        m.unlock();
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        /*md - `FILEPATH: filepath` to set filepath. By default it is `serialized/track.cfg`.*/
        if (strcmp(key, "FILEPATH") == 0) {
            setFilepath(value);
            return true;
        }

        /*md - `MAX_VARIATION: 12` to set max variation. By default it is `12`.*/
        if (strcmp(key, "MAX_VARIATION") == 0) {
            variation.props().max = atof(value);
            return true;
        }

        /*md - `EDIT_VARIATION: true` toggle to enable variation edit mode. If set to false variation will be read only. If set to true, every changes will be save before to switch to the next variation. Default is false`*/
        if (strcmp(key, "EDIT_VARIATION") == 0) {
            saveBeforeChangingVariation = strcmp(value, "true") == 0;
            return true;
        }

        return AudioPlugin::config(key, value);
    }

    void onEvent(AudioEventType event, bool isPlaying) override
    {
        if (event == AudioEventType::AUTOSAVE) {
            if (!initialized) {
                m.lock();
                hydrate();
                m.unlock();
                initialized = true;
            } else {
                serialize();
            }
        }
    }

    void serialize()
    {
        m.lock();

        FILE* file = fopen(filepath.c_str(), "w");
        for (AudioPlugin* plugin : props.audioPluginHandler->plugins) {
            if ((track == -1 || track == plugin->track) && plugin->serializable) {
                fprintf(file, "# %s\n", plugin->name);
                plugin->serialize(file, "\n");
            }
        }
        fclose(file);

        m.unlock();
    }

    void hydrate()
    {
        FILE* file = fopen(filepath.c_str(), "r");
        if (!file) {
            printf("Hydration file not found: %s\n", filepath.c_str());
            return;
        }

        AudioPlugin* plugin = NULL;
        char _line[1024];
        while (fgets(_line, 1024, file)) {
            char* line = rtrim(_line, '\n');
            if (strlen(line) > 0) {
                if (line[0] == '#') {
                    plugin = props.audioPluginHandler->getPluginPtr(line + 2, track);
                    if (plugin && !plugin->serializable) {
                        printf("Cannot hydrate plugin: %s (not serializable)\n", plugin->name);
                        plugin = NULL;
                    }
                } else if (plugin) {
                    plugin->hydrate(line);
                }
            }
        }
        fclose(file);
    }

    void hydrate(std::string value)
    {
        // Do not hydrate this plugin, else it would make a loop

        // Set variation only on first hydration
        if (!initialized && value.find("VARIATION ") != std::string::npos) {
            variation.setFloat(std::stoi(value.substr(10)));
        }
    }

    enum DATA_ID {
        SET_FILEPATH,
        SERIALIZE,
        HYDRATE,
        GET_VARIATION,
        GET_VARIATION_PATH,
        SAVE_VARIATION,
        LOAD_VARIATION,
    };

    /*md **Data ID**: */
    uint8_t getDataId(std::string name) override
    {
        /*md - `SET_FILEPATH` set filepath */
        if (name == "SET_FILEPATH")
            return DATA_ID::SET_FILEPATH;
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
        return atoi(name.c_str());
    }

    std::string dataStr;
    void* data(int id, void* userdata = NULL)
    {
        switch (id) {
        case DATA_ID::SET_FILEPATH: {
            if (userdata) {
                setFilepath((char*)userdata);
            }
            return NULL;
        }
        case DATA_ID::SERIALIZE:
            data(0, userdata);
            serialize();
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
                int id = *(int16_t*)userdata;
                m.lock();
                hydrate();
                m.unlock();
            }
            return NULL;
        }
        }
        return NULL;
    }
};

#endif
