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
#include "plugins/audio/audioPlugin.h"

class Track {
public:
    uint8_t id;
    uint8_t status = 2;
    std::vector<AudioPlugin*> plugins;
    std::thread thread;
    float* buffer;
    std::mutex mtx;
    std::condition_variable cv;
    std::unique_lock<std::mutex> lock = std::unique_lock(mtx);
    std::set<Track*> dependentTracks;
    std::set<Track*> trackDependencies;
    std::vector<Track*> tracks;

    std::mutex& tracksMtx;

    Track(uint8_t id, float* buffer, std::mutex& tracksMtx)
        : id(id)
        , buffer(buffer)
        , tracksMtx(tracksMtx)
    {
    }

    void init(std::vector<Track*> tracks)
    {
        this->tracks = tracks;

        // Set track dependencies
        std::set<uint8_t> dependencies;
        for (AudioPlugin* plugin : plugins) {
            std::set<uint8_t> pluginDependencies = plugin->trackDependencies();
            dependencies.insert(pluginDependencies.begin(), pluginDependencies.end());
        }
        for (uint8_t dependency : dependencies) {
            for (Track* dependencyTrack : tracks) {
                if (dependencyTrack->id == dependency) {
                    dependencyTrack->dependentTracks.insert(this);
                    trackDependencies.insert(dependencyTrack);
                }
            }
        }
    }

    void initMasterTrack()
    {
        for (Track* track : tracks) {
            if (track->trackDependencies.size() == 0) {
                // printf(">>>>>>>>>>>>>>>>>>>>>>> Track %d has no dependencies\n", track->id);
                dependentTracks.insert(track);
                // track->status = 0;
            }
        }
        thread = std::thread(&Track::loop, this);
        onFinish();
    }

    void startThread()
    {
        // printf("-------\n");
        // printf("Track %d has %ld tracks waiting for it:\n", id, dependentTracks.size());
        // for (Track* dependent : dependentTracks) {
        //     printf("  - %d\n", dependent->id);
        // }
        // printf("Track %d has %ld depencies:\n", id, trackDependencies.size());
        // for (Track* dependency : trackDependencies) {
        //     printf("  - %d\n", dependency->id);
        // }

        thread = std::thread(&Track::loop, this);
    }

    bool allDependenciesFinished()
    {
        for (Track* dependency : trackDependencies) {
            if (dependency->status != 2) {
                return false;
            }
        }
        return true;
    }

    void onFinish()
    {
        // printf("Track %d finished generating sample\n", id);
        tracksMtx.lock();
        status = 2;
        for (Track* track : dependentTracks) {
            if (track->allDependenciesFinished()) {
                track->status = 0;
            } else {
                // printf("Track %d depedency from track %d still have none finished dependencies\n", track->id, id);
            }

            if (track->status == 0) {
                track->status = 1;
                // printf("Track %d notifying track %d that it is finished [%d]\n", id, track->id, status);
                track->cv.notify_one();
                // status = 0;
            } else {
                // printf("Track %d depedency from track %d cannot be starting because of status %d\n", track->id, id, track->status);
            }
        }
        tracksMtx.unlock();
    }

    void loop()
    {
        while (isRunning) {
            cv.wait(lock, [&] { return status == 1; });

            // printf("Track %d generating sample\n", id);
            for (AudioPlugin* plugin : plugins) {
                plugin->sample(buffer);
            }
            onFinish();
        }
    }
};

#endif
