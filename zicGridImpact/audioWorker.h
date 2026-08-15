#pragma once

#include <alsa/asoundlib.h>
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory>
#include <thread>

#include "studio.h"
#include "gridState.h"
#include "log.h"

inline std::atomic<bool> keep_running { true };

inline snd_pcm_t* audioInit()
{
    snd_pcm_t* pcm_handle = nullptr;
    int err = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        logWarn("ALSA open error: %s (will run silent UI)", snd_strerror(err));
        return nullptr;
    }

    err = snd_pcm_set_params(pcm_handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, 44100, 1, 20000);
    if (err < 0) {
        logWarn("ALSA set_params error: %s", snd_strerror(err));
        snd_pcm_close(pcm_handle);
        return nullptr;
    }
    return pcm_handle;
}

inline void audioWorker(snd_pcm_t* pcm_h)
{
    const int bufferFrames = 64;
    float floatBuffer[bufferFrames * 2];
    int16_t pcmBuffer[bufferFrames * 2];

    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(studio.audioMutex);

            for (int i = 0; i < bufferFrames; ++i) {
                bool trigKick = false, trigSynth1 = false, trigSynth2 = false;
                float velocity = 1.0f;

                if (studio.seq.tick(trigKick, trigSynth1, trigSynth2, velocity)) {
                    if (trigKick) {
                        studio.kick.noteOn(36, velocity);
                    }
                    if (trigSynth1 && !gridState.isSynth1Muted) {
                        studio.synth1.noteOn(studio.seq.synth1Notes[studio.seq.currentStep], velocity);
                    }
                    if (trigSynth2 && !gridState.isSynth2Muted) {
                        studio.synth2.noteOn(studio.seq.synth2Notes[studio.seq.currentStep], velocity);
                    }
                }

                // Sample engines
                float kickS = studio.kick.sample();
                float s1S = gridState.isSynth1Muted ? 0.0f : studio.synth1.sample();
                float s2S = gridState.isSynth2Muted ? 0.0f : studio.synth2.sample();

                // Mixer process
                float mixed = studio.mixer.process(kickS, s1S, studio.synth1.delaySend.value, s2S, studio.synth2.delaySend.value);

                // Scatter FX process
                float left = mixed;
                float right = mixed;
                studio.scatter.process(left, right);

                floatBuffer[i * 2] = left;
                floatBuffer[i * 2 + 1] = right;
            }
        }

        for (int i = 0; i < bufferFrames * 2; ++i) {
            float s = std::clamp(floatBuffer[i], -1.0f, 1.0f);
            pcmBuffer[i] = static_cast<int16_t>(s * 32767.0f);
        }

        if (pcm_h) {
            int w = snd_pcm_writei(pcm_h, pcmBuffer, bufferFrames);
            if (w < 0) {
                w = snd_pcm_recover(pcm_h, (int)w, 0);
                if (w < 0) {
                    std::cerr << "ALSA: " << snd_strerror((int)w) << "\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
        } else {
            std::this_thread::sleep_for(std::chrono::microseconds(1400));
        }
    }
}
