// zicRack/audioWorker.h
#pragma once

#include <alsa/asoundlib.h>
#include <atomic>
#include <iostream>
#include <mutex>
#include <vector>

#include "helpers/clamp.h"
#include "zicRack/studio.h"

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

    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            std::fill(buf.begin(), buf.end(), 0);

            for (uint32_t f = 0; f < num_frames; f++) {
                if (studio.isPlaying) {
                    studio.sampleCounter = studio.sampleCounter + 1.0;
                    if (studio.sampleCounter >= studio.samplesPerStep) {
                        studio.sampleCounter = 0;
                        studio.currentStep = (studio.currentStep + 1) % SEQ_STEPS;
                        for (auto& trk : studio.tracks) {
                            auto& step = trk->sequence[studio.currentStep];
                            if (step.active && !trk->isMuted && rnd.pct() <= step.condition) {
                                trk->engine->noteOn(step.note, step.velocity);
                                trk->noteSamplesRemaining = (uint32_t)(step.len * studio.samplesPerStep);
                            }
                        }
                    }
                }

                for (auto& trk : studio.tracks) {
                    if (trk->noteSamplesRemaining > 0 && --trk->noteSamplesRemaining == 0) {
                        trk->engine->noteOff(-1);
                    }
                }

                float drumOuput = 0.f;
                float synthOutput = 0.f;
                for (auto& trk : studio.tracks) {
                    float s = trk->engine->sample() * (trk->isMuted ? 0.f : trk->volume);
                    s = trk->eq.process(s); // EQ (post-EQ samples go to spectrum)
                    trk->spectrum.push(s); // spectrum ring buffer

                    if (f == 0) {
                        std::lock_guard<std::mutex> hl(trk->historyMtx);
                        trk->history.push_back(std::abs(s));
                        trk->history.pop_front();
                    }

                    if (trk->type == TrackType::TRACK_TYPE_DRUM) {
                        drumOuput += s;
                    } else {
                        synthOutput += s;
                    }
                }

                float out = studio.masterScatter.process(drumOuput, synthOutput, studio.activeScatterMode, studio.samplesPerStep);
                out = studio.filter.process(out);
                out = studio.compressor.process(out);

                int16_t v = (int16_t)(CLAMP(out, -1.f, 1.f) * 32767.f / (MAX_TRACKS / 2));
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