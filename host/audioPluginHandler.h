#ifndef _AUDIO_PLUGIN_HANDLER_H_
#define _AUDIO_PLUGIN_HANDLER_H_

using namespace std;

#include <stdexcept>
#include <string>
#include <thread>

#include "../helpers/trim.h"
#include "../log.h"
#include "../plugins/audio/audioPlugin.h"
#include "../plugins/audio/lookupTable.h"
#include "def.h"
#include "midiMapping.h"

/*md
## Global and generic config

> [!NOTE]
> Generic config are configs that can be used by any plugin. This must be passed after a plugin has been defined.

*/

class AudioPluginHandler : public AudioPluginHandlerInterface {
public:
    struct PluginAlias {
        string name;
        std::function<AudioPlugin*(AudioPlugin::Props& props, char* name)> allocator;
    };
    std::vector<PluginAlias> pluginAliases;

protected:
    LookupTable lookupTable;

    AudioPlugin::Props pluginProps = { debug, SAMPLE_RATE, APP_CHANNELS, this, MAX_TRACKS, &lookupTable };

    std::vector<MidiMapping> midiMapping;

    // When reached the maximum, there might be a tempo issue
    // However, to reach it, it is almost impossible...
    uint64_t clockCounter = 0;
    bool playing = false;

    struct MidiNoteEvent {
        uint8_t channel;
        NoteTarget target;
    };
    std::vector<MidiNoteEvent> midiNoteEvents;

    std::thread autoSaveThread;

    static AudioPluginHandler* instance;
    AudioPluginHandler() { }

    bool assignMidiPluginChannel(char* value)
    {
        uint8_t channel = atoi(value);
        if (channel < 1 || channel > 16) {
            APP_INFO("Invalid midi note channel, set to 1\n");
            channel = 1;
        }
        midiNoteEvents.push_back({ (uint8_t)(channel - 1), { .plugin = plugins.back() } });
        APP_INFO("[%s] Midi plugin channel set to %d\n", plugins.back()->name, channel);
        return true;
    }

    bool assignMidiTrackChannel(char* value)
    {
        int16_t track = atoi(strtok(value, " "));
        uint8_t channel = atoi(strtok(NULL, " "));
        if (channel < 1 || channel > 16) {
            APP_INFO("Invalid midi note channel, set to 1\n");
            channel = 1;
        }
        midiNoteEvents.push_back({ (uint8_t)(channel - 1), { track } });
        APP_INFO("[%s] Midi track channel set to %d\n", plugins.back()->name, channel);
        return true;
    }

    bool assignMidiMapping(char* value)
    {
        // split value by space
        char* pluginKey = strtok(value, " ");
        char* msg0 = strtok(NULL, " ");
        char* msg1 = strtok(NULL, " ");
        char* msg2 = strtok(NULL, " ");

        if (msg0 == NULL || msg1 == NULL) {
            APP_INFO("Invalid midi mapping\n");
            return false;
        }

        uint8_t size = msg2 == NULL ? 2 : 3;
        uint8_t valuePosition = msg1[0] == 'x' && msg1[1] == 'x' ? 2 : 3;
        uint8_t msg0Int = strtol(msg0, NULL, 16);
        uint8_t msg1Int = strtol(msg1, NULL, 16);

        // try to assign value to last plugin
        int valueIndex = plugins.back()->getValueIndex(pluginKey);
        if (valueIndex != -1) {
            midiMapping.push_back({ plugins.back(), valueIndex, size, valuePosition, msg0Int, msg1Int });
            APP_INFO("[%s] Midi mapping assigned: %s\n", plugins.back()->name, pluginKey);
            return true;
        }
        return false;
    }

public:
    static AudioPluginHandler& get()
    {
        if (!instance) {
            instance = new AudioPluginHandler();
        }
        return *instance;
    }

    AudioPlugin* getPluginPtr(const char* name, int16_t track = -1)
    {
        for (AudioPlugin* plugin : plugins) {
            if (strcmp(plugin->name, name) == 0 && (track == -1 || plugin->track == track)) {
                return plugin;
            }
        }
        return NULL;
    }

    AudioPlugin& getPlugin(const char* name, int16_t track = -1)
    {
        AudioPlugin* plugin = getPluginPtr(name, track);
        if (!plugin) {
            throw std::runtime_error("Could not find plugin " + std::string(name) + " on track " + std::to_string(track));
        }
        return *plugin;
    }

    void loop()
    {
        while (isRunning) {
            float buffer[MAX_TRACKS] = { 0.0f };
            for (AudioPlugin* plugin : plugins) {
                plugin->sample(buffer);
            }
            // then in props pass max tracks
            // then as part of audio plugin, setTrack and by default to 0

            // float s = 0.0f;
            // for (Plugin& plugin : plugins) {
            //     s = plugin.instance->sample(s);
            // }
        }
    }

    void loadPluginAlias(char* value)
    {
        PluginAlias pluginAllocator;

        char* name = strtok(value, " ");
        char* path = strtok(NULL, " ");

        void* handle = dlopen(path, RTLD_LAZY);
        if (!handle) {
            logError("Cannot load audio library %s [%s]: %s\n", path, name, dlerror());
            return;
        }

        dlerror();
        pluginAllocator.name = name;
        pluginAllocator.allocator = [handle](AudioPlugin::Props& props, char* name) {
            void* allocator = dlsym(handle, "allocator");
            return ((AudioPlugin * (*)(AudioPlugin::Props & props, char* name)) allocator)(props, name);
        };
        const char* dlsym_error = dlerror();
        if (dlsym_error) {
            logError("Cannot load symbol: %s\n", dlsym_error);
            dlclose(handle);
            return;
        }
        pluginAliases.push_back(pluginAllocator);

        logInfo("audio plugin alias loaded: %s\n", name);
    }

    void loadPlugin(char* value)
    {
        char* name = strtok(value, " ");
        char * pathPtr = strtok(NULL, " ");
        string path = pathPtr ? pathPtr : name;

        // if path end by .so load it as plugin
        if (path.substr(path.length() - 3) == ".so") {
            loadPlugin(name, path.c_str());
        } else {
            // look to alias if plugins is already loaded and allocate it
            for (PluginAlias& pluginAllocator : pluginAliases) {
                printf("pluginAllocator.name %s ==? path %s\n", pluginAllocator.name.c_str(), path.c_str());
                if (pluginAllocator.name == path) {
                    plugins.push_back(pluginAllocator.allocator(pluginProps, name));
                    logInfo("audio plugin loaded: %s\n", plugins.back()->name);
                    return;
                }
            }
            logWarn("Cannot find audio plugin alias %s [%s]\n", path.c_str(), name);
        }
    }

    void loadPlugin(char* name, const char* path)
    {
        void* handle = dlopen(path, RTLD_LAZY);

        if (!handle) {
            logWarn("Cannot open audio library %s [%s]: %s\n", path, name, dlerror());
            return;
        }

        dlerror();
        void* allocator = (AudioPlugin*)dlsym(handle, "allocator");
        const char* dlsym_error = dlerror();
        if (dlsym_error) {
            logWarn("Cannot load symbol: %s\n", dlsym_error);
            dlclose(handle);
            return;
        }

        AudioPlugin* instance = ((AudioPlugin * (*)(AudioPlugin::Props & props, char* name)) allocator)(pluginProps, name);
        logInfo("audio plugin loaded: %s\n", instance->name);

        // plugin.instance->set(0, 0.1f);
        // printf("---> getParamKey: %d\n", plugin.instance->getParamKey("volume"));

        plugins.push_back(instance);
    }

    bool config(char* key, char* value)
    {
        if (plugins.size() > 0) {
            if (plugins.back()->config(key, value)) {
                return true;
                /*md - `MIDI_CC: CUTOFF b0 4c xx` assign a midi CC command to a given plugin value (this is a generic config). */
            } else if (strcmp(key, "MIDI_CC") == 0) {
                return assignMidiMapping(value);
                /*md - `MIDI_CHANNEL: 1` assign a midi channel to a given plugin for a note on/off (this is a generic config). */
            } else if (strcmp(key, "MIDI_CHANNEL") == 0) {
                return assignMidiPluginChannel(value);
                /*md - `TRACK_MIDI_CHANNEL: track channel` assign a midi channel to a given track for a note on/off, e.g. `TRACK_MIDI_CHANNEL: 1 2` */
            } else if (strcmp(key, "TRACK_MIDI_CHANNEL") == 0) {
                return assignMidiTrackChannel(value);
            }
        }
        return false;
    }

    bool midi(std::vector<unsigned char>* message)
    {
        for (MidiMapping& mapping : midiMapping) {
            if (mapping.handle(message)) {
                return true;
            }
        }
        return false;
    }

    void noteOn(uint8_t note, float velocity, NoteTarget target)
    {
        if (target.plugin) {
            target.plugin->noteOn(note, velocity);
        } else {
            for (AudioPlugin* plugin : plugins) {
                if (target.track == -1 || target.track == plugin->track) {
                    plugin->noteOn(note, velocity);
                }
            }
        }
    }

    void noteOff(uint8_t note, float velocity, NoteTarget target)
    {
        if (target.plugin) {
            target.plugin->noteOff(note, velocity);
        } else {
            for (AudioPlugin* plugin : plugins) {
                if (target.track == -1 || target.track == plugin->track) {
                    plugin->noteOff(note, velocity);
                }
            }
        }
    }

    void noteOn(uint8_t channel, uint8_t note, float velocity)
    {
        // printf("-------------- noteOn %d %d %f\n", channel, note, velocity);
        for (MidiNoteEvent& target : midiNoteEvents) {
            if (target.channel == channel) {
                if (velocity == 0) {
                    noteOff(note, velocity, target.target);
                } else {
                    noteOn(note, velocity, target.target);
                }
            }
        }
    }

    void noteOff(uint8_t channel, uint8_t note, float velocity)
    {
        // printf("------------- noteOff %d %d %f\n", channel, note, velocity);
        for (MidiNoteEvent& target : midiNoteEvents) {
            if (target.channel == channel) {
                noteOn(note, velocity, target.target);
            }
        }
    }

    void clockTick()
    {
        if (playing) {
            clockCounter++;
            for (AudioPlugin* plugin : plugins) {
                plugin->onClockTick(&clockCounter);
            }
        }
    }

    bool isPlaying()
    {
        return playing;
    }

    bool isStopped()
    {
        return !playing && clockCounter == 0;
    }

    void sendEvent(AudioEventType event)
    {
        switch (event) {
        case AudioEventType::START:
            playing = true;
            break;

        case AudioEventType::STOP:
            playing = false;
            clockCounter = 0;
            break;

        case AudioEventType::PAUSE:
            playing = false;
            break;

        case AudioEventType::TOGGLE_PLAY_PAUSE:
            playing = !playing;
            break;
        }
        for (AudioPlugin* plugin : plugins) {
            plugin->onEvent(event);
        }
    }

    void start()
    {
        sendEvent(AudioEventType::START);
    }

    void stop()
    {
        sendEvent(AudioEventType::STOP);
    }

    void pause()
    {
        sendEvent(AudioEventType::PAUSE);
    }

    void startAutoSave(uint32_t msInterval)
    {
        if (msInterval < 50) {
            msInterval = 50;
        }

        autoSaveThread = std::thread([this, msInterval]() {
            APP_PRINT("Starting autosave thread with interval %d ms\n", msInterval);

            // Wait a bit first, else it crash...
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            while (isRunning) {
                sendEvent(AudioEventType::AUTOSAVE);
                std::this_thread::sleep_for(std::chrono::milliseconds(msInterval));
            }
        });

        // Save a last time before to exit
        sendEvent(AudioEventType::AUTOSAVE);
    }
};

AudioPluginHandler* AudioPluginHandler::instance = NULL;

#endif