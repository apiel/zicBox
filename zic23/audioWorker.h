// zic23/audioWorker.h
#pragma once

#include <alsa/asoundlib.h>
#include <atomic>
#include <iostream>
#include <mutex>
#include <vector>

#include "helpers/clamp.h"
#include "zic23/studio.h"

snd_pcm_t* audioInit()
{
    snd_pcm_t* pcm_h;
    int err = snd_pcm_open(&pcm_h, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        std::cerr << "Audio open error: " << snd_strerror(err) << std::endl;
        return nullptr;
    }
    snd_pcm_set_params(pcm_h, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED,
        2, SAMPLE_RATE, 1, 20000);
    return pcm_h;
}

void audioWorker(snd_pcm_t* pcm)
{
    if (!pcm) return;

    const size_t num_frames = 256;
    std::vector<int16_t> buffer_pcm(num_frames * 2);

    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            std::fill(buffer_pcm.begin(), buffer_pcm.end(), 0);

            for (uint32_t f = 0; f < num_frames; f++) {
                if (studio.isPlaying) {
                    studio.sampleCounter = studio.sampleCounter + 1.0;
                    if (studio.sampleCounter >= studio.samplesPerStep) {
                        studio.sampleCounter = 0;
                        studio.currentStep = (studio.currentStep + 1) % SEQ_STEPS;

                        for (auto& trk : studio.tracks) {
                            auto& step = trk->sequence[studio.currentStep];
                            if (step.active && !trk->isMuted) {
                                if (rnd.pct() <= step.condition) {
                                    trk->engine->noteOn(step.note, step.velocity);
                                    trk->noteSamplesRemaining = (uint32_t)(step.len * studio.samplesPerStep);
                                }
                            }
                        }
                    }
                }

                for (auto& trk : studio.tracks) {
                    if (trk->noteSamplesRemaining > 0) {
                        trk->noteSamplesRemaining--;
                        if (trk->noteSamplesRemaining == 0)
                            trk->engine->noteOff(-1);
                    }
                }

                for (auto& trk : studio.tracks) {
                    float s = trk->engine->sample();

                    // EQ processing
                    s = trk->eq.process(s);

                    // Feed post-EQ sample into the spectrum analyser
                    trk->spectrum.push(s);

                    if (f == 0) {
                        std::lock_guard<std::mutex> hLock(trk->historyMtx);
                        trk->history.push_back(std::abs(s));
                        trk->history.pop_front();
                    }

                    float p = s * (trk->isMuted ? 0.0f : trk->volume);
                    int16_t v = (int16_t)(CLAMP(p, -1.f, 1.f) * 32767.f / (MAX_TRACKS / 2));
                    buffer_pcm[f * 2] += v;
                    buffer_pcm[f * 2 + 1] += v;
                }
            }
        }

        sf::Int64 written = snd_pcm_writei(pcm, buffer_pcm.data(), num_frames);
        if (written < 0) {
            written = snd_pcm_recover(pcm, (int)written, 0);
            if (written < 0) {
                std::cerr << "ALSA recovery failed: " << snd_strerror((int)written) << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }
}