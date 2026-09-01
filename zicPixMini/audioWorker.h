#pragma once

#include "audio/Compressor.h"
#include "audio/Scatter.h"
#include "helpers/clamp.h"
#include "studio.h"

#include <alsa/asoundlib.h>
#include <atomic>
#include <chrono>
#include <cmath>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <thread>
#include <vector>

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

struct MasterAudioFX {
    Scatter scatter;
    Compressor compressor;

    MasterAudioFX(float sampleRate = 44100.0f)
        : compressor(sampleRate)
    {
    }
};

inline void audioWorker(snd_pcm_t* pcm)
{
    setAudioThreadRealtime();
    if (!pcm) return;

    const size_t num_frames = 256;
    std::vector<int16_t> buf(num_frames * 2);
    studio.updateClock();

    MasterAudioFX fx(studio.sampleRate);

    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            std::fill(buf.begin(), buf.end(), 0);
            double samplesPerStep = (studio.sampleRate * 60.0) / (studio.bpm.load() * 4.0);

            float framePeakD = 0.0f;
            float framePeakS1 = 0.0f;
            float framePeakS2 = 0.0f;
            float framePeakM = 0.0f;

            for (uint32_t f = 0; f < num_frames; f++) {
                if (studio.isPlaying) {
                    studio.stepCounter++;
                    if (studio.stepCounter >= studio.samplesPerStep) {
                        studio.stepCounter = 0;
                        int cur = studio.currentStep.load();
                        int nextStep = (cur + 1) % SEQ_STEPS_MINI;
                        studio.currentStep.store(nextStep);

                        // Trigger Track 0 (TribeDrums - 4 Lanes)
                        for (int r = 0; r < 4; r++) {
                            if (studio.trackDrums.rowEnabled[r]) {
                                auto& stpDrum = studio.trackDrums.sequence[r][nextStep];
                                if (stpDrum.active && !studio.trackDrums.isMuted) {
                                    studio.trackDrums.drums.noteOn(r, stpDrum.note, stpDrum.velocity);
                                    studio.drumPulseTrigger.store(true);
                                }
                            }
                        }

                        // Trigger Track 1 (DriftSynth1)
                        auto& stp1 = studio.trackSynth1.sequence[nextStep];
                        if (stp1.active && !studio.trackSynth1.isMuted) {
                            studio.trackSynth1.engine.trigger();
                            studio.synth1PulseTrigger.store(true);
                        }

                        // Trigger Track 2 (DriftWavetable)
                        auto& stp2 = studio.trackSynth2.sequence[nextStep];
                        if (stp2.active && !studio.trackSynth2.isMuted) {
                            studio.trackSynth2.engine.trigger();
                            studio.synth2PulseTrigger.store(true);
                        }
                    }
                }

                // Render samples for drums and synths
                float sDrums = studio.trackDrums.drums.sample() * (studio.trackDrums.isMuted ? 0.0f : studio.trackDrums.volume);
                float sSynth1 = studio.trackSynth1.engine.sample() * (studio.trackSynth1.isMuted ? 0.0f : studio.trackSynth1.volume);
                float sSynth2 = studio.trackSynth2.engine.sample() * (studio.trackSynth2.isMuted ? 0.0f : studio.trackSynth2.volume);

                float master = sDrums + sSynth1 + sSynth2;

                // Scatter FX & Master Compressor
                master = fx.scatter.process(master, samplesPerStep);
                master = fx.compressor.process(master);

                // Soft clip master output
                master = std::tanh(master * 0.85f);

                studio.pushMasterSample(master);

                framePeakD = std::max(framePeakD, std::abs(sDrums));
                framePeakS1 = std::max(framePeakS1, std::abs(sSynth1));
                framePeakS2 = std::max(framePeakS2, std::abs(sSynth2));
                framePeakM = std::max(framePeakM, std::abs(master));

                int16_t pcmVal = static_cast<int16_t>(CLAMP(master, -1.0f, 1.0f) * 32767.0f);
                buf[f * 2] = pcmVal;
                buf[f * 2 + 1] = pcmVal;
            }

            studio.peakDrums.store(std::max(studio.peakDrums.load() * 0.88f, framePeakD));
            studio.peakSynth1.store(std::max(studio.peakSynth1.load() * 0.88f, framePeakS1));
            studio.peakSynth2.store(std::max(studio.peakSynth2.load() * 0.88f, framePeakS2));
            studio.peakMaster.store(std::max(studio.peakMaster.load() * 0.88f, framePeakM));
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
