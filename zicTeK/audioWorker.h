#pragma once

#include <alsa/asoundlib.h>
#include <atomic>
#include <chrono>
#include <cmath>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <thread>
#include <vector>

#include "helpers/clamp.h"
#include "zicTeK/studio.h"

extern std::atomic<bool> keep_running;

inline snd_pcm_t* audioInit(unsigned int sampleRate = 44100)
{
    snd_pcm_t* pcm_h = nullptr;
    int err = snd_pcm_open(&pcm_h, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        std::cerr << "Audio open error: " << snd_strerror(err) << std::endl;
        return nullptr;
    }
    snd_pcm_set_params(pcm_h, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, sampleRate, 1, 20000);
    return pcm_h;
}

inline void setAudioThreadRealtime()
{
    sched_param sch {};
    sch.sched_priority = 30;
    int rc = pthread_setschedparam(pthread_self(), SCHED_FIFO, &sch);
    if (rc != 0) {
        std::cout << "[Audio] Note: Unable to set SCHED_FIFO realtime priority (run with sudo/CAP_SYS_NICE for lowest latency)\n";
    } else {
        std::cout << "[Audio] SCHED_FIFO priority 30 set successfully\n";
    }
}

inline void audioWorker(snd_pcm_t* pcm)
{
    setAudioThreadRealtime();
    if (!pcm) return;

    const size_t num_frames = 256;
    std::vector<int16_t> buf(num_frames * 2);
    studio.updateClock();

    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            std::fill(buf.begin(), buf.end(), 0);

            for (uint32_t f = 0; f < num_frames; f++) {
                if (studio.isPlaying) {
                    studio.stepCounter++;
                    if (studio.stepCounter >= studio.samplesPerStep) {
                        studio.stepCounter = 0;
                        int nextStep = (studio.currentStep.load() + 1) % SEQ_STEPS_TEK;
                        studio.currentStep.store(nextStep);

                        auto& stp = studio.track0.sequence[nextStep];
                        if (stp.active && !studio.track0.isMuted) {
                            studio.track0.kick.noteOn(stp.note, stp.velocity);
                            studio.kickPulseTrigger.store(true);
                        }
                    }
                }

                // Render sample for Track 0 (ImpactKick)
                float s = studio.track0.kick.sample() * (studio.track0.isMuted ? 0.0f : studio.track0.volume);

                // Soft clip master output
                s = std::tanh(s * 1.1f);

                int16_t pcmVal = static_cast<int16_t>(CLAMP(s, -1.0f, 1.0f) * 32767.0f);
                buf[f * 2] = pcmVal;
                buf[f * 2 + 1] = pcmVal;
            }
        }

        snd_pcm_sframes_t w = snd_pcm_writei(pcm, buf.data(), num_frames);
        if (w < 0) {
            w = snd_pcm_recover(pcm, static_cast<int>(w), 0);
            if (w < 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }
    }
}
