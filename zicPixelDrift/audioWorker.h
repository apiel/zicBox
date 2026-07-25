#pragma once

#include "kickBody.h"
#include "mixer.h"
#include "sequencer.h"
#include "ui.h"

#include <alsa/asoundlib.h>
#include <atomic>
#include <cmath>
#include <iostream>
#include <vector>

extern std::atomic<bool> keep_running;
extern KickBody kickEngine;
extern Sequencer seqEngine;
extern Mixer mixerEngine;
extern UiPixelDrift* globalUiPtr;

inline snd_pcm_t* audioInit()
{
    snd_pcm_t* pcm_handle = nullptr;
    int err;

    if ((err = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        std::cerr << "Cannot open audio device default: " << snd_strerror(err) << std::endl;
        return nullptr;
    }

    if ((err = snd_pcm_set_params(pcm_handle,
             SND_PCM_FORMAT_S16_LE,
             SND_PCM_ACCESS_RW_INTERLEAVED,
             2, // Stereo
             44100, // 44.1 kHz
             1, // Soft resample
             32000)) // 32ms latency
        < 0) {
        std::cerr << "PCM set params error: " << snd_strerror(err) << std::endl;
        return nullptr;
    }

    return pcm_handle;
}

inline void audioWorker(snd_pcm_t* pcm_handle)
{
    const int bufferFrames = 256;
    std::vector<int16_t> buffer(bufferFrames * 2);

    while (keep_running) {
        for (int i = 0; i < bufferFrames; ++i) {
            bool isKickTrigger = false;
            float velocity = 1.0f;

            if (seqEngine.tick(isKickTrigger, velocity)) {
                if (isKickTrigger && globalUiPtr && !globalUiPtr->isKickMuted) {
                    kickEngine.trigger(velocity);
                }
            }

            float kickSample = kickEngine.sample() * mixerEngine.kickVol;
            float masterOut = mixerEngine.processMaster(kickSample);

            int16_t s16 = (int16_t)std::clamp(masterOut * 32767.0f, -32768.0f, 32767.0f);
            buffer[i * 2] = s16; // Left channel
            buffer[i * 2 + 1] = s16; // Right channel
        }

        if (pcm_handle) {
            snd_pcm_sframes_t frames = snd_pcm_writei(pcm_handle, buffer.data(), bufferFrames);
            if (frames < 0) {
                frames = snd_pcm_recover(pcm_handle, (int)frames, 0);
            }
        }
    }
}
