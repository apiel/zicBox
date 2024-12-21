#ifndef _AUDIO_TRACK_H_
#define _AUDIO_TRACK_H_

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

#include "def.h"
#include "plugins/audio/audioPlugin.h"

class Track {
public:
    uint8_t id;
    bool sampling = false;
    std::vector<AudioPlugin*> plugins;
    std::thread thread;
    float* buffer;
    std::mutex& mtx;
    std::condition_variable& cv;
    std::vector<Track*> trackDependencies;

    Track(uint8_t id, float* buffer, std::mutex& mtx, std::condition_variable& cv)
        : id(id)
        , buffer(buffer)
        , mtx(mtx)
        , cv(cv)
    {
    }

    bool shouldStartSampling()
    {
        if (!sampling) {
            return false;
        }
        for (Track* dependency : trackDependencies) {
            if (dependency->sampling) {
                return false;
            }
        }
        return true;
    }

    void loop()
    {
        while (isRunning) {
            std::unique_lock lock(mtx);
            cv.wait(lock, [&] { return shouldStartSampling(); });

            // printf("Track %d generating sample\n", id);
            for (AudioPlugin* plugin : plugins) {
                plugin->sample(buffer);
            }
            // printf("Track %d finished\n", id);

            sampling = false;
            cv.notify_all();
        }
    }
};

#endif