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
                        int cur = studio.currentStep.load();
                        int nextStep = (cur + 1) % SEQ_STEPS_TEK;
                        studio.currentStep.store(nextStep);

                        // Trigger Track 0 (Massive Kick)
                        int r0 = nextStep / 16;
                        if (studio.track0.rowEnabled[r0]) {
                            auto& stp0 = studio.track0.sequence[nextStep];
                            if (stp0.active && !studio.track0.isMuted) {
                                studio.track0.engine.noteOn(stp0.note, stp0.velocity);
                                studio.kickPulseTrigger.store(true);
                            }
                        }

                        // Trigger Track 1 (TeKSynth)
                        int r1 = nextStep / 16;
                        if (studio.track1.rowEnabled[r1]) {
                            auto& stp1 = studio.track1.sequence[nextStep];
                            if (stp1.active && !studio.track1.isMuted) {
                                studio.track1.engine.noteOn(stp1.note, stp1.velocity);
                                studio.synthPulseTrigger.store(true);
                            }
                        }

                        // Trigger Track 2 (Tekno Tribe Drums - 4 Lanes)
                        int stepInBar = nextStep % 16;
                        for (int r = 0; r < 4; r++) {
                            if (studio.track2.rowEnabled[r]) {
                                auto& stpDrum = studio.track2.sequence[r * 16 + stepInBar];
                                if (stpDrum.active && !studio.track2.isMuted) {
                                    studio.track2.drums.noteOn(r, stpDrum.note, stpDrum.velocity);
                                    studio.drumPulseTrigger.store(true);
                                }
                            }
                        }
                    }
                }

                // Render Track 0 (TeKKick), Track 1 (TeKSynth), and Track 2 (TribeDrums)
                float s0 = studio.track0.engine.sample() * (studio.track0.isMuted ? 0.0f : studio.track0.volume);
                float s1 = studio.track1.engine.sample() * (studio.track1.isMuted ? 0.0f : studio.track1.volume);
                float s2 = studio.track2.drums.sample() * (studio.track2.isMuted ? 0.0f : studio.track2.volume);

                // --- Sidechain Ducking Envelope Follower ---
                float kickAbs = std::abs(s0);
                float alpha = (kickAbs > studio.sidechainEnv) ? 0.999f : 0.9995f;
                studio.sidechainEnv = alpha * studio.sidechainEnv + (1.0f - alpha) * kickAbs;

                // --- Sidechain Ducking Gain on Synth ---
                float duckDepth = studio.sidechainDuckAmount.load();
                float duckGain = 1.0f - (studio.sidechainEnv * duckDepth * 1.5f);
                duckGain = CLAMP(duckGain, 0.15f, 1.0f);

                s1 *= duckGain;

                // --- Synth High-Pass Filter (80Hz Sub-Cut to clear Kick Sub Space) ---
                float hpCutoffW = 2.0f * (float)M_PI * 80.0f / studio.sampleRate;
                float hpAlpha = 1.0f / (1.0f + hpCutoffW);
                studio.hpBuf = hpAlpha * (studio.hpBuf + s1 - studio.prevSynthSample);
                studio.prevSynthSample = s1;
                s1 = studio.hpBuf;

                // Sum and soft clip master output
                float master = std::tanh((s0 + s1 + s2) * 0.8f);

                int16_t pcmVal = static_cast<int16_t>(CLAMP(master, -1.0f, 1.0f) * 32767.0f);
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
