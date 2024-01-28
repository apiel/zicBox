#ifndef _SERIALIZE_TRACK_H_
#define _SERIALIZE_TRACK_H_

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
    /*md - `VARIATION` switch between different track serialization varaitions (clip). WIP */
    Val& variation = val(0.0f, "VARIATION", { "Variation", .max = 12.0f }, [&](auto p) { setVariation(p.value); });

    SerializeTrack(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
    }

    void sample(float* buf)
    {
    }

    void setVariation(float value)
    {
        // should it use a mutex to avoid race conditions with serialization thread?
        // also need to avoid that new variation get save to current file...
        variation.setFloat(value);
        // if variation is different than current variation
        // then we can hydrate
        // but first we need to save the current state, to variation folder
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        /*md - `FILEPATH` to set filepath. By default it is `serialized/track.cfg`.*/
        if (strcmp(key, "FILEPATH") == 0) {
            setFilepath(value);
            return true;
        }

        /*md - `MAX_VARIATION` to set max variation. By default it is `12`.*/
        if (strcmp(key, "MAX_VARIATION") == 0) {
            variation.props().max = atof(value);
            return true;
        }

        return AudioPlugin::config(key, value);
    }

    void onEvent(AudioEventType event)
    {
        if (event == AudioEventType::AUTOSAVE) {
            if (!initialized) {
                hydrate();
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
        for (AudioPluginHandlerInterface::Plugin& plugin : props.audioPluginHandler->plugins) {
            if ((track == -1 || track == plugin.instance->track) && plugin.instance->serializable) {
                fprintf(file, "# %s\n", plugin.instance->name);
                plugin.instance->serialize(file, "\n");
            }
        }
        fclose(file);

        m.unlock();
    }

    void hydrate()
    {
        m.lock();

        FILE* file = fopen(filepath.c_str(), "r");
        if (!file) {
            printf("Hydration file not found: %s\n", filepath.c_str());
            m.unlock();
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

        m.unlock();
    }

    void* data(int id, void* userdata = NULL)
    {
        switch (id) {
        case 0: {
            if (userdata) {
                setFilepath((char*)userdata);
            }
            return NULL;
        }
        case 1:
            data(0, userdata);
            serialize();
            return NULL;
        case 2:
            data(0, userdata);
            hydrate();
            return NULL;
        }
        return NULL;
    }
};

#endif
