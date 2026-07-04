#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

#include "def.h"
#include "log.h"
#include "plugins/audio/audioPlugin.h"

class Track {
public:
    uint8_t id;
    std::vector<AudioPlugin*> plugins;
    int pluginsSize = 0;
    std::thread thread;
    float* buffer;
    std::condition_variable cv;
    // bool processing = false;
    std::atomic<bool> processing = false;
    int16_t maxTracks;

    std::condition_variable& masterCv;

    Track(uint8_t id, float* buffer, std::condition_variable& masterCv, int16_t maxTracks)
        : id(id)
        , buffer(buffer)
        , masterCv(masterCv)
        , maxTracks(maxTracks)
    {
    }

    std::set<uint8_t> getDependencies()
    {
        std::set<uint8_t> dependencies;
        for (AudioPlugin* plugin : plugins) {
            std::set<uint8_t> pluginDependencies = plugin->trackDependencies();
            if (pluginDependencies.size() > 0) {
                dependencies.insert(pluginDependencies.begin(), pluginDependencies.end());
            }
        }
        return dependencies;
    }

    bool hasDependencies()
    {
        for (AudioPlugin* plugin : plugins) {
            if (plugin->trackDependencies().size() > 0) {
                return true;
            }
        }
        return false;
    }

    void init(std::vector<Track*> tracks, bool isMaster)
    {
        pluginsSize = plugins.size();
        // Only start a thread if track doesn't have any dependency on another tracks
        // All mixing and master track will be done in the main loop
        //
        // Master track should never start in a thread, else it would cause some glitching noise in audio output
        if (!hasDependencies() && !isMaster) {
            logDebug(">>> Track %d has no dependency, start a thread", id);
            // There is no dependency, start a thread
            thread = std::thread([this] { loop(); });
            pthread_setname_np(thread.native_handle(), ("track_" + std::to_string(id)).c_str());
        }

        // for (int i = 0; i < plugins.size(); i++) {
        //     AudioPlugin* plugin = plugins[i];
        //     printf("Track %d plugin %d %s\n", id, i, plugin->name);
        // }
    }

    void loop()
    {
        std::mutex mtx;
        std::unique_lock lock(mtx);

        while (isRunning) {
            cv.wait(lock, [&] { return processing == true; });
            // process();
            for (uint8_t i = 0; i < 128; i++) {
                process(i);
            }
            processing = false;
            masterCv.notify_one();
        }
    }

    void process(uint8_t index)
    {
        process(buffer + index * maxTracks);
    }
    // void process(uint8_t index)
    // {
    //     for (int i = 0; i < pluginsSize; i++) {
    //         AudioPlugin* plugin = plugins[i];
    //         plugin->sample(buffer + index * maxTracks);
    //     }
    // }

    void process(float* buf)
    {
        // Even if compiler should optimze it, it seems that
        // using iterator is still slower than for (int i = 0; i < pluginsSize; i++)
        //
        // for (AudioPlugin* plugin : plugins) {
        //     plugin->sample(buf);
        // }

        for (int i = 0; i < pluginsSize; i++) {
            AudioPlugin* plugin = plugins[i];
            plugin->sample(buf);
        }

        // plugins[0]->sample(buf);
        // plugins[1]->sample(buf);
        // plugins[2]->sample(buf);
        // plugins[3]->sample(buf);
        // plugins[4]->sample(buf);
        // plugins[5]->sample(buf);
        // plugins[6]->sample(buf);
        // plugins[7]->sample(buf);
    }

    void noteOn(uint8_t note, float velocity)
    {
        for (AudioPlugin* plugin : plugins) {
            plugin->noteOn(note, velocity, NULL);
        }
    }

    void noteOff(uint8_t note, float velocity)
    {
        for (AudioPlugin* plugin : plugins) {
            plugin->noteOff(note, velocity, NULL);
        }
    }
};

// Track 0 plugin 0 Drum23
// Track 0 plugin 1 Sequencer
// Track 0 plugin 2 Distortion
// Track 0 plugin 3 MMFilter
// Track 0 plugin 4 Volume
// Track 0 plugin 5 AudioOutput
// Track 0 plugin 6 SerializeTrack
// Track 0 plugin 7 Tempo
