#ifndef _AUDIO_PLUGIN_H_
#define _AUDIO_PLUGIN_H_

#include <cstdlib>
#include <stdint.h>
#include <string.h>
#include <vector>

#include "valueInterface.h"

class AudioPlugin;

class AudioPluginHandlerInterface {
public:
    struct Plugin {
        void* handle;
        AudioPlugin* instance;
    };

    std::vector<Plugin> plugins;

    virtual AudioPlugin& getPlugin(const char* name) = 0;
};

class AudioPlugin {
public:
    char name[64];
    uint16_t maxTracks;
    uint16_t track = 0;

    AudioPluginHandlerInterface* audioPluginHandler;
    int (*debug)(const char* format, ...);

    struct Props {
        int (*debug)(const char* format, ...);
        uint64_t sampleRate;
        uint8_t channels;
        AudioPluginHandlerInterface* audioPluginHandler;
        uint16_t maxTracks;
    };

    AudioPlugin(Props& props, char* _name)
        : maxTracks(props.maxTracks)
        , audioPluginHandler(props.audioPluginHandler)
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
            if (track >= maxTracks) {
                track = maxTracks - 1;
            }
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

    enum Status {
        STOP,
        START,
        PAUSE
    };
    virtual void onStatus(Status status)
    {
    }
};

#endif