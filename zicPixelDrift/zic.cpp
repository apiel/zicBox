#include <alsa/asoundlib.h>
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory>
#include <pthread.h>
#include <sched.h>
#include <thread>

#ifdef IS_RPI
#define AUDIO_FOLDER std::string("data/audio")
#else
#define AUDIO_FOLDER std::string("../data/audio")
#endif

#include "audioWorker.h"
#include "audio/engines/DriftKick.h"
#include "log.h"
#include "mixer.h"
#include "sequencer.h"
#include "audio/engines/DriftSynth1.h"
#include "audio/engines/DriftSynth2.h"
#include "ui.h"

std::atomic<bool> keep_running { true };

AudioWorker worker(44100.0f);
UiPixelDrift* globalUiPtr = nullptr;

#ifdef DRAW_SMFL
#include "runtimeDesktopSFML.h"
#else
#include "runtimeHardware.h"
#endif

namespace {
void audioThreadLoop(snd_pcm_t* pcm_h)
{
    const int bufferFrames = 64;
    float floatBuffer[bufferFrames * 2];
    int16_t pcmBuffer[bufferFrames * 2];

    while (keep_running) {
        worker.processAudioBlock(floatBuffer, bufferFrames);

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

snd_pcm_t* audioInit()
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

void setThreadRealtime(pthread_t thread, int priority, const char* name)
{
    sched_param sch {};
    sch.sched_priority = priority;
    int rc = pthread_setschedparam(thread, SCHED_FIFO, &sch);
    if (rc != 0) {
        logWarn("Unable to set realtime priority for %s (need root/CAP_SYS_NICE)", name);
    } else {
        logInfo("Realtime priority set for %s", name);
    }
}
}

int main()
{
    logInfo("Starting zicPixelDrift (Tekno Kick & Space Engine)");

    snd_pcm_t* pcm_h = audioInit();
    pthread_setname_np(pthread_self(), "zicPixel_UI");

    UiPixelDrift ui(worker);
    globalUiPtr = &ui;

    std::thread aThread(audioThreadLoop, pcm_h);
    pthread_setname_np(aThread.native_handle(), "zicPixel_Audio");
    setThreadRealtime(aThread.native_handle(), 30, "audio thread");

    Styles appStyles = {
        .screen = { 320, 176 },
        .margin = 2,
        .colors = { { 15, 15, 18 }, { 255, 255, 255 }, { 120, 120, 130 }, { 0, 180, 255 }, { 10, 10, 12 }, { 28, 28, 32 }, { 35, 35, 40 } }
    };
    auto drawer = std::make_unique<Draw>(appStyles);
    bool needFullRedraw = true;

#ifdef DRAW_SMFL
    runDesktopSFML(*drawer, ui, needFullRedraw);
#else
    runHardware(*drawer, ui, needFullRedraw);
#endif

    keep_running = false;
    aThread.join();
    if (pcm_h) snd_pcm_close(pcm_h);

    logInfo("zicPixelDrift shutdown cleanly.");
    return 0;
}
