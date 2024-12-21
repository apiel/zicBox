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
    bool sampling = true;
    std::vector<AudioPlugin*> plugins;
    std::thread thread;
    float* buffer;
    std::mutex& mtx;
    std::condition_variable& cv;
    std::vector<Track*> trackDependencies;
    std::vector<Track*> tracks;

    Track(uint8_t id, float* buffer, std::mutex& mtx, std::condition_variable& cv)
        : id(id)
        , buffer(buffer)
        , mtx(mtx)
        , cv(cv)
    {
    }

    void init(std::vector<Track*> tracks)
    {
        this->tracks = tracks;

        // Set Track dependencies
        for (AudioPlugin* plugin : plugins) {
            std::vector<uint8_t> dependencies = plugin->trackDependencies();
            for (uint8_t dependency : dependencies) {
                for (Track* dependencyTrack : tracks) {
                    if (dependencyTrack->id == dependency) {
                        trackDependencies.push_back(dependencyTrack);
                    }
                }
            }
        }

        // remove duplicates
        std::sort(trackDependencies.begin(), trackDependencies.end());
        trackDependencies.erase(std::unique(trackDependencies.begin(), trackDependencies.end()), trackDependencies.end());

        // printf("Track dependencies for track %d [%ld]:\n", id, trackDependencies.size());
        for (Track* dependency : trackDependencies) {
            // printf("  - %d\n", dependency->id);
        }

        // start thread
        thread = std::thread(&Track::loop, this);
    }

    bool checkCondition()
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

    void notify()
    {
        // std::unique_lock lock(mtx);
        for (Track* track : tracks) {
            if (track->sampling) {
                // If there is a track that is currently sampling, dont reset all
                // lock.unlock();
                cv.notify_all();
                return;
            }
        }
        // lock.unlock();
        // No more tracks are currently sampling, reset all
        for (Track* track : tracks) {
            track->sampling = true;
        }
        cv.notify_all();
    }

    void loop()
    {
        while (isRunning) {
            std::unique_lock lock(mtx);
            cv.wait(lock, [&] { return checkCondition(); });
            lock.unlock();

            // printf("Track %d generating sample\n", id);
            for (AudioPlugin* plugin : plugins) {
                plugin->sample(buffer);
            }
            // printf("Track %d finished\n", id);

            sampling = false;
            notify();
        }
    }
};

#endif