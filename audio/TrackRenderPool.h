#pragma once

#include <algorithm>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

// Per-track, per-audio-frame control event produced by the sequencer stage
// and consumed by the audio render stage.
struct TrackFrameEvent {
    bool loadClip = false;
    int clipIdx = -1;

    bool noteOn = false;
    int note = 0;
    float velocity = 0.f;
    uint32_t noteLenSamples = 0;
};


// Generic multi-thread track renderer.
//
// `TrackT` is your track type.
// `renderTrackFn` receives one track, its contiguous event slice,
// frame count and a worker-local mix buffer to accumulate into.
//
// Notes:
// - `renderTrackFn` is called concurrently on different tracks.
// - `localMix` is worker-local, so no lock is needed for mixing there.
// - final merge into `mixed` is done serially in `render()`.
template <typename TrackT>
class TrackRenderPool {
public:
    using RenderTrackFn = std::function<void(TrackT&, const TrackFrameEvent*, size_t, std::vector<float>&)>;

    TrackRenderPool(size_t workerCount, RenderTrackFn renderTrackFn)
        : workerCount_(workerCount)
        , renderTrackFn_(std::move(renderTrackFn))
    {
        partialMix_.resize(workerCount_);
        for (size_t i = 0; i < workerCount_; ++i) {
            workers_.emplace_back([this, i]() { workerLoop(i); });
        }
    }

    ~TrackRenderPool()
    {
        {
            std::lock_guard<std::mutex> lock(jobMutex_);
            stop_ = true;
            ++jobGeneration_;
        }
        jobCv_.notify_all();
        for (auto& t : workers_) {
            if (t.joinable()) t.join();
        }
    }

    void render(
        const std::vector<TrackT*>& tracks,
        const std::vector<TrackFrameEvent>& events,
        size_t numFrames,
        std::vector<float>& mixed)
    {
        if (workerCount_ == 0 || tracks.empty() || numFrames == 0 || !renderTrackFn_) {
            std::fill(mixed.begin(), mixed.end(), 0.f);
            return;
        }

        for (size_t w = 0; w < workerCount_; ++w) {
            partialMix_[w].assign(numFrames, 0.f);
        }

        {
            std::lock_guard<std::mutex> lock(jobMutex_);
            tracks_ = &tracks;
            events_ = &events;
            numFrames_ = numFrames;
            pendingWorkers_ = workerCount_;
            ++jobGeneration_;
        }
        jobCv_.notify_all();

        {
            std::unique_lock<std::mutex> lock(jobMutex_);
            doneCv_.wait(lock, [this]() { return pendingWorkers_ == 0; });
        }

        std::fill(mixed.begin(), mixed.end(), 0.f);
        for (size_t w = 0; w < workerCount_; ++w) {
            const auto& src = partialMix_[w];
            for (size_t f = 0; f < numFrames; ++f) {
                mixed[f] += src[f];
            }
        }
    }

private:
    void workerLoop(size_t workerIdx)
    {
        uint64_t seenGeneration = 0;

        for (;;) {
            const std::vector<TrackT*>* tracks = nullptr;
            const std::vector<TrackFrameEvent>* events = nullptr;
            size_t numFrames = 0;
            uint64_t generation = 0;

            {
                std::unique_lock<std::mutex> lock(jobMutex_);
                jobCv_.wait(lock, [this, seenGeneration]() {
                    return stop_ || jobGeneration_ != seenGeneration;
                });

                if (stop_) return;

                generation = jobGeneration_;
                tracks = tracks_;
                events = events_;
                numFrames = numFrames_;
            }

            auto& localMix = partialMix_[workerIdx];
            std::fill(localMix.begin(), localMix.end(), 0.f);

            const size_t trackCount = tracks->size();
            for (size_t t = workerIdx; t < trackCount; t += workerCount_) {
                TrackT* trk = (*tracks)[t];
                if (!trk) continue;

                const TrackFrameEvent* trackEvents = &(*events)[t * numFrames];
                renderTrackFn_(*trk, trackEvents, numFrames, localMix);
            }

            {
                std::lock_guard<std::mutex> lock(jobMutex_);
                seenGeneration = generation;
                if (pendingWorkers_ > 0 && --pendingWorkers_ == 0) {
                    doneCv_.notify_one();
                }
            }
        }
    }

    size_t workerCount_ = 0;
    RenderTrackFn renderTrackFn_;
    std::vector<std::thread> workers_;
    std::vector<std::vector<float>> partialMix_;

    std::mutex jobMutex_;
    std::condition_variable jobCv_;
    std::condition_variable doneCv_;

    const std::vector<TrackT*>* tracks_ = nullptr;
    const std::vector<TrackFrameEvent>* events_ = nullptr;
    size_t numFrames_ = 0;

    size_t pendingWorkers_ = 0;
    uint64_t jobGeneration_ = 0;
    bool stop_ = false;
};
