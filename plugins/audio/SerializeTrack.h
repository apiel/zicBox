#ifndef _SERIALIZE_TRACK_H_
#define _SERIALIZE_TRACK_H_

#include "../../helpers/trim.h"
#include "audioPlugin.h"

class SerializeTrack : public AudioPlugin {
protected:
    std::string filepath = "serialize/track.cfg";

public:
    SerializeTrack(AudioPlugin::Props& props, char* _name)
        : AudioPlugin(props, _name)
    {
    }

    void sample(float* buf)
    {
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "FILEPATH") == 0) {
            filepath = value;
            return true;
        }
        return AudioPlugin::config(key, value);
    }

    void serialize()
    {
        FILE* file = fopen(filepath.c_str(), "w");
        for (AudioPluginHandlerInterface::Plugin& plugin : props.audioPluginHandler->plugins) {
            if ((track == -1 || track == plugin.instance->track) && plugin.instance->serializable) {
                fprintf(file, "# %s\n", plugin.instance->name);
                plugin.instance->serialize(file, "\n");
            }
        }
        fclose(file);
    }

    void hydrate()
    {
        FILE* file = fopen(filepath.c_str(), "r");
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

    void* data(int id, void* userdata = NULL)
    {
        switch (id) {
        case 0:
            serialize();
            return NULL;
        case 1:
            hydrate();
            return NULL;
        case 2:
            filepath = (char *)userdata;
            return NULL;
        }
        return NULL;
    }
};

#endif
