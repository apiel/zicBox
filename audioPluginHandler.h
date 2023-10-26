#ifndef _AUDIO_PLUGIN_HANDLER_H_
#define _AUDIO_PLUGIN_HANDLER_H_

#include <stdexcept>

#include "def.h"
#include "midiMapping.h"
#include "plugins/audio/audioPlugin.h"

class AudioPluginHandler : public AudioPluginHandlerInterface {
protected:
    AudioPlugin::Props pluginProps = { debug, SAMPLE_RATE, APP_CHANNELS, this, MAX_TRACKS };

    std::vector<MidiMapping> midiMapping;

    static AudioPluginHandler* instance;
    AudioPluginHandler() { }

    bool assignMidiMapping(char* key, char* value)
    {
        // split value by space
        char* msg0 = strtok(value, " ");
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
        int valueIndex = plugins.back().instance->getValueIndex(key);
        if (valueIndex != -1) {
            midiMapping.push_back({ plugins.back().instance, valueIndex, size, valuePosition, msg0Int, msg1Int });
            APP_INFO("[%s] Midi mapping assigned: %s\n", plugins.back().instance->name, key);
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

    AudioPlugin& getPlugin(const char* name)
    {
        for (Plugin& plugin : plugins) {
            if (strcmp(plugin.instance->name, name) == 0) {
                return *plugin.instance;
            }
        }
        throw std::runtime_error("Could not find plugin " + std::string(name));
    }

    void loop()
    {
        while (isRunning) {
            float buffer[MAX_TRACKS] = { 0.0f };
            for (Plugin& plugin : plugins) {
                plugin.instance->sample(buffer);
            }
            // then in props pass max tracks
            // then as part of audio plugin, setTrack and by default to 0

            // float s = 0.0f;
            // for (Plugin& plugin : plugins) {
            //     s = plugin.instance->sample(s);
            // }
        }
    }

    void loadPlugin(char* value)
    {
        Plugin plugin;

        char* name = strtok(value, " ");
        char* path = strtok(NULL, " ");

        plugin.handle = dlopen(path, RTLD_LAZY);

        if (!plugin.handle) {
            APP_PRINT("Cannot open library: %s\n", dlerror());
            return;
        }

        dlerror();
        void* allocator = (AudioPlugin*)dlsym(plugin.handle, "allocator");
        const char* dlsym_error = dlerror();
        if (dlsym_error) {
            APP_PRINT("Cannot load symbol: %s\n", dlsym_error);
            dlclose(plugin.handle);
            return;
        }

        plugin.instance = ((AudioPlugin * (*)(AudioPlugin::Props & props, char* name)) allocator)(pluginProps, name);
        APP_PRINT("audio plugin loaded\n");
        APP_PRINT("plugin: %s\n", plugin.instance->name);

        // plugin.instance->set(0, 0.1f);
        // printf("---> getParamKey: %d\n", plugin.instance->getParamKey("volume"));

        plugins.push_back(plugin);
    }

    bool config(char* key, char* value)
    {
        if (plugins.size() > 0) {
            if (plugins.back().instance->config(key, value)) {
                return true;
            } else {
                return assignMidiMapping(key, value);
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

    void clockTick()
    {
        for (Plugin& plugin : plugins) {
            plugin.instance->onClockTick();
        }
    }

    void start()
    {
        for (Plugin& plugin : plugins) {
            plugin.instance->onStatus(AudioPlugin::Status::START);
        }
    }

    void stop()
    {
        for (Plugin& plugin : plugins) {
            plugin.instance->onStatus(AudioPlugin::Status::STOP);
        }
    }

    void pause()
    {
        for (Plugin& plugin : plugins) {
            plugin.instance->onStatus(AudioPlugin::Status::PAUSE);
        }
    }
};

AudioPluginHandler* AudioPluginHandler::instance = NULL;

#endif