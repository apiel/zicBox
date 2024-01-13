#ifndef _AUDIO_PLUGIN_H_
#define _AUDIO_PLUGIN_H_

#include <cstdlib>
#include <stdint.h>
#include <string.h>
#include <vector>

#include "valueInterface.h"

class AudioPlugin;

    enum AudioEventType {
        STOP,
        START,
        PAUSE,
        AUTOSAVE
    };

class AudioPluginHandlerInterface {
public:
    struct Plugin {
        void* handle;
        AudioPlugin* instance;
    };

    std::vector<Plugin> plugins;

    virtual AudioPlugin* getPluginPtr(const char* name, int16_t track = -1) = 0;
    virtual AudioPlugin& getPlugin(const char* name, int16_t track = -1) = 0;
    virtual void clockTick() = 0;
    virtual void sendEvent(AudioEventType event) = 0;
};

class AudioPlugin {
public:
    struct Props {
        int (*debug)(const char* format, ...);
        uint64_t sampleRate;
        uint8_t channels;
        AudioPluginHandlerInterface* audioPluginHandler;
        int16_t maxTracks;
    };

protected:
    Props props;

public:
    char name[64];
    int16_t track = 0;
    bool serializable = true;

    int (*debug)(const char* format, ...);

    AudioPlugin(Props& props, char* _name)
        : props(props)
        , debug(props.debug)
    {
        strcpy(name, _name);
    }

    virtual void sample(float* buf) = 0;

    virtual ValueInterface* getValue(int valueIndex)
    {
        return NULL;
    }

    virtual ValueInterface* getValue(const char* key)
    {
        return NULL;
    }

    virtual int getValueCount()
    {
        return 0;
    }

    virtual int getValueIndex(const char* key)
    {
        return -1;
    }

    virtual void noteOn(uint8_t note, uint8_t velocity)
    {
    }

    virtual void noteOff(uint8_t note, uint8_t velocity)
    {
    }

    virtual bool config(char* key, char* value)
    {
        if (strcmp(key, "TRACK") == 0) {
            track = atoi(value);
            if (track >= props.maxTracks) {
                track = props.maxTracks - 1;
            }
            return true;
        }

        if (strcmp(key, "SERIALIZABLE") == 0) {
            serializable = strcmp(value, "true") == 0;
            return true;
        }

        return false;
    }

    virtual void* data(int id, void* userdata = NULL)
    {
        return NULL;
    }

    virtual void onClockTick()
    {
    }

    virtual void onStart()
    {
    }

    virtual void onEvent(AudioEventType event)
    {
    }

    /**
     * @brief Serialize the plugin values to a file
     *
     * @param file
     * @param separator should be used as value separator
     */
    virtual void serialize(FILE* file, std::string separator)
    {
    }

    /**
     * @brief Hydrate the plugin values from a serialized file
     */
    virtual void hydrate(std::string value)
    {
    }
};

#endif