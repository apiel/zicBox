#ifndef _AUDIO_TRACK_H_
#define _AUDIO_TRACK_H_

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
    std::thread thread;
    float* buffer;
    std::condition_variable cv;
    // bool processing = false;
    std::atomic<bool> processing = false;

    std::mutex& masterMtx;
    std::condition_variable& masterCv;

    Track(uint8_t id, float* buffer, std::mutex& masterMtx, std::condition_variable& masterCv)
        : id(id)
        , buffer(buffer)
        , masterMtx(masterMtx)
        , masterCv(masterCv)
    {
    }

    void init(std::vector<Track*> tracks)
    {
        // Only start a thread if track doesn't have any dependency on another tracks
        // All mixing and master track will be done in the main loop
        for (AudioPlugin* plugin : plugins) {
            if (plugin->trackDependencies().size() > 0) {
                return;
            }
        }
        logDebug(">>> Track %d has no dependency, start a thread", id);
        // There is no dependency, start a thread
        thread = std::thread([this] { loop(); });
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
        for (AudioPlugin* plugin : plugins) {
            plugin->sample(buffer + index * 32);
        }
    }
};

#endif
