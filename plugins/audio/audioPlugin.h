#ifndef _AUDIO_PLUGIN_H_
#define _AUDIO_PLUGIN_H_

#include <cstdlib>
#include <stdint.h>
#include <string.h>
#include <vector>

#include "lookupTable.h"
#include "valueInterface.h"

class AudioPlugin;

enum AudioEventType {
    STOP,
    START,
    PAUSE,
    TOGGLE_PLAY_PAUSE,
    AUTOSAVE,
    COUNT
};

AudioEventType getEventTypeFromName(std::string name)
{
    if (name == "STOP") {
        return AudioEventType::STOP;
    } else if (name == "START") {
        return AudioEventType::START;
    } else if (name == "PAUSE") {
        return AudioEventType::PAUSE;
    } else if (name == "TOGGLE_PLAY_PAUSE") {
        return AudioEventType::TOGGLE_PLAY_PAUSE;
    }
    return AudioEventType::COUNT;
}

class AudioPluginHandlerInterface {
public:
    std::vector<AudioPlugin*> plugins;

    virtual AudioPlugin* getPluginPtr(const char* name, int16_t track = -1) = 0;
    virtual AudioPlugin& getPlugin(const char* name, int16_t track = -1) = 0;
    virtual void clockTick() = 0;
    virtual void sendEvent(AudioEventType event) = 0;

    struct NoteTarget {
        int16_t track = -1;
        AudioPlugin* plugin = NULL;
    };
    virtual void noteOn(uint8_t note, float velocity, NoteTarget target) = 0;
    virtual void noteOff(uint8_t note, float velocity, NoteTarget target) = 0;

    virtual bool isPlaying() = 0;
    virtual bool isStopped() = 0;
};

class AudioPlugin {
public:
    struct Props {
        int (*debug)(const char* format, ...);
        uint64_t sampleRate;
        uint8_t channels;
        AudioPluginHandlerInterface* audioPluginHandler;
        int16_t maxTracks;
        LookupTable* lookupTable;
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

    virtual ValueInterface* getValue(std::string key)
    {
        return NULL;
    }

    virtual int getValueCount()
    {
        return 0;
    }

    virtual int getValueIndex(std::string key)
    {
        return -1;
    }

    virtual void noteOn(uint8_t note, float velocity)
    {
    }

    virtual void noteOff(uint8_t note, float velocity)
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

    virtual uint8_t getDataId(std::string name)
    {
        return atoi(name.c_str());
    }

    virtual void onClockTick(uint64_t* clockCounter)
    {
    }

    virtual void onStart()
    {
    }

    virtual void onEvent(AudioEventType event, bool playing)
    {
    }

    virtual std::vector<uint8_t> trackDependencies() { return {}; }

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