// zicXYv2/audioWorker.h
#pragma once

#include <alsa/asoundlib.h>
#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "audio/TrackRenderPool.h"
#include "helpers/clamp.h"
#include "zicXYv2/studio.h"

void loadClip(Track& trk, int clipIdx);

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
    std::vector<float> tapeBuf(num_frames, 0.f);
    std::vector<float> mixed(num_frames, 0.f);

    const size_t hw = std::thread::hardware_concurrency() == 0 ? 2 : std::thread::hardware_concurrency();
    // Reserve cores for UI + audio thread to avoid contention spikes during redraw/input.
    const size_t maxWorkersByHw = (hw > 2) ? (hw - 2) : 1;
    const size_t workers = std::min<size_t>(4, std::max<size_t>(1, maxWorkersByHw));

    auto renderTrackFn = [](Track& trk, const TrackFrameEvent* trackEvents, size_t numFrames, std::vector<float>& localMix) {
        if (!trk.engine) return;

        float firstSampleAbs = 0.f;
        bool hasFirstSample = false;

        for (size_t f = 0; f < numFrames; ++f) {
            const TrackFrameEvent& ev = trackEvents[f];

            if (ev.loadClip && ev.clipIdx >= 0) {
                loadClip(trk, ev.clipIdx);
                trk.pendingClipIdx = -1;
            }

            if (ev.noteOn) {
                trk.engine->noteOn(ev.note, ev.velocity);
                trk.noteSamplesRemaining = ev.noteLenSamples;
            }

            if (trk.noteSamplesRemaining > 0 && --trk.noteSamplesRemaining == 0) {
                trk.engine->noteOff(-1);
            }

            const float s = trk.engine->sample() * (trk.isMuted ? 0.f : trk.volume);
            if (!hasFirstSample) {
                firstSampleAbs = std::abs(s);
                hasFirstSample = true;
            }
            localMix[f] += s;
        }

        if (hasFirstSample) {
            std::lock_guard<std::mutex> hl(trk.historyMtx);
            trk.history.push_back(firstSampleAbs);
            trk.history.pop_front();
        }
    };

    TrackRenderPool<Track> renderPool(workers, renderTrackFn);
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
                float out = studio.masterScatter.process(mixed[f], studio.samplesPerStep);
                out = studio.filter.process(out);
                out = studio.compressor.process(out);
                tapeBuf[f] = out;

                int16_t v = (int16_t)(CLAMP(out, -1.f, 1.f) * 32767.f / (MAX_TRACKS / 2)) * studio.volume;
                buf[f * 2] += v;
                buf[f * 2 + 1] += v;
            }

            studio.tape.process(tapeBuf.data(), num_frames, SAMPLE_RATE);
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