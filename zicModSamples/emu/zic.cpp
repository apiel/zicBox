#include <alsa/asoundlib.h>
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <sched.h>
#include <thread>
#include <vector>

#include "displayView.h"
#include "runtimeDesktopSFML.h"
#include "zicApp.h"

std::atomic<bool> keep_running(true);
std::mutex audioMutex;

const unsigned int SAMPLE_RATE = 44100;
const size_t NUM_FRAMES = 256;

ZicApp app(SAMPLE_RATE);
DisplayView displayView;

void setThreadRealtime(pthread_t thread, int priority, const char* name)
{
    sched_param sch {};
    sch.sched_priority = priority;
    int rc = pthread_setschedparam(thread, SCHED_FIFO, &sch);
    if (rc != 0) {
        std::cerr << "Unable to set realtime priority for " << name << " (need CAP_SYS_NICE/root)\n";
    } else {
        std::cout << "Realtime priority set for " << name << "\n";
    }
}

snd_pcm_t* audioInit()
{
    snd_pcm_t* pcm_h = nullptr;
    int err = snd_pcm_open(&pcm_h, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        std::cerr << "Audio open error: " << snd_strerror(err) << "\n";
        return nullptr;
    }
    snd_pcm_set_params(pcm_h, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, SAMPLE_RATE, 1, 20000);
    return pcm_h;
}

void audioWorker(snd_pcm_t* pcm)
{
    if (!pcm) return;
    std::vector<int16_t> buffer(NUM_FRAMES * 2);

    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(audioMutex);
            for (size_t f = 0; f < NUM_FRAMES; ++f) {
                float sample = app.renderMasterSample();
                int16_t v = (int16_t)(std::clamp(sample, -1.0f, 1.0f) * 32767.0f);
                buffer[f * 2] = v;     // Left
                buffer[f * 2 + 1] = v; // Right
            }
        }

        int w = snd_pcm_writei(pcm, buffer.data(), NUM_FRAMES);
        if (w < 0) {
            w = snd_pcm_recover(pcm, (int)w, 0);
            if (w < 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }
}

int main()
{
    std::cout << "Starting zicModDrums Desktop Simulator (320x170 Touch UI & ALSA Audio)...\n";

    snd_pcm_t* pcm_h = audioInit();
    if (!pcm_h) {
        std::cerr << "Failed to initialize ALSA audio. Exiting.\n";
        return 1;
    }

    pthread_setname_np(pthread_self(), "zicDrums_UI");

    std::thread audioThread(audioWorker, pcm_h);
    pthread_setname_np(audioThread.native_handle(), "zicDrums_Audio");
    setThreadRealtime(audioThread.native_handle(), 35, "audio thread");

    Styles appStyles = {
        .screen = { 320, 170 },
        .margin = 2,
        .colors = { { 15, 18, 24, 255 }, { 255, 255, 255, 255 }, { 120, 120, 130, 255 }, { 0, 200, 255, 255 }, { 10, 10, 12, 255 }, { 28, 28, 32, 255 }, { 35, 35, 40, 255 } }
    };
    auto drawer = std::make_unique<Draw>(appStyles);

    runDesktopSFML(*drawer, app, displayView, keep_running);

    keep_running = false;
    audioThread.join();
    snd_pcm_close(pcm_h);

    std::cout << "zicModDrums stopped cleanly.\n";
    return 0;
}
