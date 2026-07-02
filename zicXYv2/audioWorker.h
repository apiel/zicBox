// zicXYv2/audioWorker.h
#pragma once

#include <alsa/asoundlib.h>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "helpers/clamp.h"
#include "zicXYv2/studio.h"

void loadClip(Track& trk, int clipIdx);

struct TrackFrameEvent {
    bool loadClip = false;
    int clipIdx = -1;
    bool noteOn = false;
    int note = 0;
    float velocity = 0.f;
    uint32_t noteLenSamples = 0;
};

class TrackRenderPool {
public:
    explicit TrackRenderPool(size_t workerCount)
        : workerCount_(workerCount)
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
        const std::vector<Track*>& tracks,
        const std::vector<TrackFrameEvent>& events,
        size_t numFrames,
        std::vector<float>& mixed)
    {
        if (workerCount_ == 0 || tracks.empty() || numFrames == 0) {
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
            const std::vector<Track*>* tracks = nullptr;
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
                Track* trk = (*tracks)[t];
                if (!trk || !trk->engine) continue;

                float firstSampleAbs = 0.f;
                bool hasFirstSample = false;

                for (size_t f = 0; f < numFrames; ++f) {
                    const TrackFrameEvent& ev = (*events)[t * numFrames + f];

                    if (ev.loadClip && ev.clipIdx >= 0) {
                        loadClip(*trk, ev.clipIdx);
                        trk->pendingClipIdx = -1;
                    }

                    if (ev.noteOn) {
                        trk->engine->noteOn(ev.note, ev.velocity);
                        trk->noteSamplesRemaining = ev.noteLenSamples;
                    }

                    if (trk->noteSamplesRemaining > 0 && --trk->noteSamplesRemaining == 0) {
                        trk->engine->noteOff(-1);
                    }

                    const float s = trk->engine->sample() * (trk->isMuted ? 0.f : trk->volume);
                    if (!hasFirstSample) {
                        firstSampleAbs = std::abs(s);
                        hasFirstSample = true;
                    }
                    localMix[f] += s;
                }

                if (hasFirstSample) {
                    std::lock_guard<std::mutex> hl(trk->historyMtx);
                    trk->history.push_back(firstSampleAbs);
                    trk->history.pop_front();
                }
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
    std::vector<std::thread> workers_;
    std::vector<std::vector<float>> partialMix_;

    std::mutex jobMutex_;
    std::condition_variable jobCv_;
    std::condition_variable doneCv_;

    const std::vector<Track*>* tracks_ = nullptr;
    const std::vector<TrackFrameEvent>* events_ = nullptr;
    size_t numFrames_ = 0;

    size_t pendingWorkers_ = 0;
    uint64_t jobGeneration_ = 0;
    bool stop_ = false;
};

snd_pcm_t* audioInit()
{
    snd_pcm_t* pcm_h;
    int err = snd_pcm_open(&pcm_h, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        std::cerr << "Audio open error: " << snd_strerror(err) << std::endl;
        return nullptr;
    }
    snd_pcm_set_params(pcm_h, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, SAMPLE_RATE, 1, 20000);
    return pcm_h;
}

void audioWorker(snd_pcm_t* pcm)
{
    if (!pcm) return;

    const size_t num_frames = 256;
    std::vector<int16_t> buf(num_frames * 2);
    std::vector<float> mixed(num_frames, 0.f);

    const size_t hw = std::thread::hardware_concurrency() == 0 ? 2 : std::thread::hardware_concurrency();
    const size_t workers = std::min<size_t>(4, std::max<size_t>(1, hw));

    TrackRenderPool renderPool(workers);
    std::vector<Track*> trackPtrs;
    std::vector<TrackFrameEvent> events;

    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            std::fill(buf.begin(), buf.end(), 0);

            trackPtrs.clear();
            trackPtrs.reserve(studio.tracks.size());
            for (auto& trk : studio.tracks) {
                trackPtrs.push_back(trk.get());
            }

            const size_t trackCount = trackPtrs.size();
            events.assign(trackCount * num_frames, TrackFrameEvent {});

            for (size_t f = 0; f < num_frames; ++f) {
                if (!studio.isPlaying) continue;

                studio.sampleCounter = studio.sampleCounter + 1.0;
                if (studio.sampleCounter >= studio.samplesPerStep) {
                    studio.sampleCounter = 0;
                    studio.currentStep = (studio.currentStep + 1) % SEQ_STEPS;

                    const bool wrapped = (studio.currentStep == 0);
                    const int curStep = studio.currentStep;

                    for (size_t t = 0; t < trackCount; ++t) {
                        Track* trk = trackPtrs[t];
                        auto& ev = events[t * num_frames + f];

                        if (wrapped && trk->pendingClipIdx >= 0) {
                            // loadClip will save current clip and set activeClipIdx
                            ev.loadClip = true;
                            ev.clipIdx = trk->pendingClipIdx;
                        }

                        auto& step = trk->sequence[curStep];
                        if (step.active && !trk->isMuted && rnd.pct() <= step.condition) {
                            ev.noteOn = true;
                            ev.note = step.note;
                            ev.velocity = step.velocity;
                            ev.noteLenSamples = (uint32_t)(step.len * studio.samplesPerStep);
                        }
                    }
                }
            }

            renderPool.render(trackPtrs, events, num_frames, mixed);

            for (size_t f = 0; f < num_frames; ++f) {
                float out = studio.masterScatter.process(mixed[f], studio.activeScatter, studio.samplesPerStep);
                out = studio.filter.process(out);
                out = studio.compressor.process(out);

                int16_t v = (int16_t)(CLAMP(out, -1.f, 1.f) * 32767.f / (MAX_TRACKS / 2)) * studio.volume;
                buf[f * 2] += v;
                buf[f * 2 + 1] += v;
            }
        }

        int w = snd_pcm_writei(pcm, buf.data(), num_frames);
        if (w < 0) {
            w = snd_pcm_recover(pcm, (int)w, 0);
            if (w < 0) {
                std::cerr << "ALSA: " << snd_strerror((int)w) << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }
}