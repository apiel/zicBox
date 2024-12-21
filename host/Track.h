#ifndef _AUDIO_TRACK_H_
#define _AUDIO_TRACK_H_

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

#include "plugins/audio/audioPlugin.h"

class Track {
public:
    uint8_t id;
    bool sampling = false;
    std::vector<AudioPlugin*> plugins;
    std::thread thread;
    float* buffer;
    std::atomic<bool>& isRunning;
    std::mutex& mtx;
    std::condition_variable& cv;
    std::vector<Track*> trackDependencies;

    Track(uint8_t id, float* buffer, std::atomic<bool>& isRunning, std::mutex& mtx, std::condition_variable& cv)
        : id(id)
        , buffer(buffer)
        , isRunning(isRunning)
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
            // Wait for a signal to generate the next sample
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [&] { return shouldStartSampling(); });
            }

            for (AudioPlugin* plugin : plugins) {
                plugin->sample(buffer);
            }

            // Notify that this track is done
            {
                std::unique_lock<std::mutex> lock(mtx);
                sampling = false;
                cv.notify_all();
            }
        }
    }
};

#endif