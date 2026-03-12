#pragma once

#include <alsa/asoundlib.h>
#include <atomic>
#include <iostream>
#include <mutex>
#include <vector>

#include "helpers/clamp.h"
#include "tek23/studio.h"

// Assume these exist in your Studio/Track structures:
// trk->noteSamplesRemaining: uint32_t to track duration
// trk->engine->noteOff(): method to trigger release phase

snd_pcm_t* audioInit()
{
    snd_pcm_t* pcm_h;
    int err = snd_pcm_open(&pcm_h, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        std::cerr << "Audio open error: " << snd_strerror(err) << std::endl;
        return nullptr;
    }
    // Standard CD quality: 44100Hz, Stereo, 16-bit
    snd_pcm_set_params(pcm_h, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, SAMPLE_RATE, 1, 20000);
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

                                    // Calculate duration in samples: (length in steps) * (samples per step)
                                    trk->noteSamplesRemaining = (uint32_t)(step.len * studio.samplesPerStep);
                                }
                            }
                        }
                    }
                }

                // 2. NOTE-OFF / ENVELOPE LOGIC
                // We check this every sample so that note-offs are sample-accurate
                for (auto& trk : studio.tracks) {
                    if (trk->noteSamplesRemaining > 0) {
                        trk->noteSamplesRemaining--;
                        if (trk->noteSamplesRemaining == 0) {
                            trk->engine->noteOff(-1); // -1 because we dont keep track of note as we are in monophonic, but if we would be polyphonic this would matter and we would have to say which note goes off...
                        }
                    }
                }

                for (auto& trk : studio.tracks) {
                    float s = trk->engine->sample();
                    if (f == 0) {
                        std::lock_guard<std::mutex> hLock(trk->historyMtx);
                        trk->history.push_back(std::abs(s));
                        trk->history.pop_front();
                    }
                    float p = s * (trk->isMuted ? 0.0f : trk->volume);
                    int16_t val = (int16_t)(CLAMP(p, -1.0f, 1.0f) * 32767.0f / (MAX_TRACKS / 2));
                    buffer_pcm[f * 2] += val;
                    buffer_pcm[f * 2 + 1] += val;
                }
            }
        }

        sf::Int64 frames_written = snd_pcm_writei(pcm, buffer_pcm.data(), num_frames);

        if (frames_written < 0) {
            frames_written = snd_pcm_recover(pcm, (int)frames_written, 0);
            if (frames_written < 0) {
                std::cerr << "ALSA recovery failed: " << snd_strerror((int)frames_written) << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }
}