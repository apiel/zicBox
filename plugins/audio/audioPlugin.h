#pragma once

#include "libs/nlohmann/json.hpp"
#include <cstdlib>
#include <set>
#include <stdint.h>
#include <string.h>
#include <vector>

#include "utils/lookupTable.h"
#include "valueInterface.h"

class AudioPlugin;

enum AudioEventType {
    TOGGLE_PLAY_PAUSE,
    AUTOSAVE,
    RELOAD_WORKSPACE,
    SEQ_LOOP,
    SET_ACTIVE_TRACK,
    START = 0xfa,
    PAUSE = 0xfb,
    STOP = 0xfc,
    UNKNOWN,
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
    } else if (name == "AUTOSAVE") {
        return AudioEventType::AUTOSAVE;
    } else if (name == "RELOAD_WORKSPACE") {
        return AudioEventType::RELOAD_WORKSPACE;
    } else if (name == "SEQ_LOOP") {
        return AudioEventType::SEQ_LOOP;
    } else if (name == "SET_ACTIVE_TRACK") {
        return AudioEventType::SET_ACTIVE_TRACK;
    }
    return AudioEventType::UNKNOWN;
}

class AudioPluginHandlerInterface {
public:
    std::vector<AudioPlugin*> plugins;

    virtual AudioPlugin* getPluginPtr(std::string name, int16_t track = -1) = 0;
    virtual AudioPlugin& getPlugin(std::string name, int16_t track = -1) = 0;
    virtual void sendEvent(AudioEventType event, int16_t track = -1) = 0;

    struct NoteTarget {
        int16_t track = -1;
        AudioPlugin* plugin = NULL;
    };
    virtual void noteOn(uint8_t note, float velocity, NoteTarget target) = 0;
    virtual void noteOff(uint8_t note, float velocity, NoteTarget target) = 0;
    virtual void assignPluginToMidiChannel(uint8_t channel, AudioPlugin* plugin) = 0;
    virtual void mapMidiCmd(AudioPlugin* plugin, int valueIndex, const std::string& cmd) = 0;
    virtual AudioPluginHandlerInterface& config(nlohmann::json& config) = 0;

    virtual bool isPlaying() = 0;
    virtual bool isStopped() = 0;

    virtual void loop() = 0;
};

class AudioPlugin {
public:
    enum Type {
        AUDIO = 1 << 0, // 1
        TEMPO = 1 << 1, // 2
        SEQUENCER = 1 << 2, // 4
        SYNTH = 1 << 3, // 8
        EFFECT = 1 << 4, // 16
        MIXER = 1 << 5, // 32
    };

    virtual int16_t getType() { return AudioPlugin::Type::AUDIO; }

    struct Props {
        uint64_t sampleRate;
        uint8_t channels;
        AudioPluginHandlerInterface* audioPluginHandler;
        int16_t maxTracks;
        LookupTable* lookupTable;
    };

    struct Config {
        std::string name;
        nlohmann::json& json;
        uint8_t trackId = 0;
    };

protected:
    Props props;

public:
    std::string name;
    int16_t track = 0;
    bool serializable = true;

    AudioPlugin(Props& props, Config& config)
        : props(props)
        , name(config.name)
        , track(config.trackId)
    {
        auto& json = config.json;
        serializable = json.value("serializable", serializable);

        //#md `{ "midiChannel": 1 }` assign a midi channel to the plugin
        if (json.contains("midiChannel")) {
            props.audioPluginHandler->assignPluginToMidiChannel(json["midiChannel"].get<uint8_t>(), this);
        }

        //#md `{ "midiCmd": [{"parameter": "VOLUME", "cmd": "b0 4c xx"}] }` assign a midi command to a given plugin value. `xx` is the position of the value changing in the target parameter.
        if (json.contains("b0 4c xx") && json["midiCmd"].is_array()) {
            for (nlohmann::json& cmd : json["midiCmd"]) {
                if (cmd.contains("parameter") && cmd.contains("cmd")) {
                    int valueIndex = getValueIndex(cmd["parameter"].get<std::string>());
                    if (valueIndex >= 0) {
                        props.audioPluginHandler->mapMidiCmd(this, valueIndex, cmd["cmd"].get<std::string>());
                    }
                }
            }
        }
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

    virtual void noteOn(uint8_t note, float velocity, void* userdata = NULL)
    {
    }

    virtual void noteOff(uint8_t note, float velocity, void* userdata = NULL)
    {
    }

    virtual void* data(int id, void* userdata = NULL)
    {
        return NULL;
    }

    virtual uint8_t getDataId(std::string name)
    {
        return atoi(name.c_str());
    }

    virtual void onStart()
    {
    }

    virtual void onEvent(AudioEventType event, bool playing)
    {
    }

    virtual std::set<uint8_t> trackDependencies() { return {}; }

    virtual void serializeJson(nlohmann::json& json)
    {
    }

    virtual void hydrateJson(nlohmann::json& json)
    {
    }
};

AudioPlugin::Props defaultAudioProps = {
    .sampleRate = 44100,
    .channels = 2,
    .audioPluginHandler = nullptr,
    .maxTracks = 16,
    .lookupTable = nullptr,
};
