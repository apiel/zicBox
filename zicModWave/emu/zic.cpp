#include <alsa/asoundlib.h>
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <sched.h>
#include <thread>
#include <vector>

#include "audioEngine.h"
#include "uiModWave.h"

#ifdef DRAW_SMFL
#include "runtimeDesktopSFML.h"
#endif

// Thread synchronization
std::atomic<bool> keep_running(true);
std::mutex audioMutex;

const unsigned int SAMPLE_RATE = 44100;
const size_t NUM_FRAMES = 256;

// Global application state
UiModWave ui(SAMPLE_RATE);

// Set realtime priority for audio thread
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

// ALSA Playback Initialization
snd_pcm_t* audioInit()
{
    snd_pcm_t* pcm_h;
    int err = snd_pcm_open(&pcm_h, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        std::cerr << "Audio open error: " << snd_strerror(err) << "\n";
        return nullptr;
    }
    snd_pcm_set_params(pcm_h, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, SAMPLE_RATE, 1, 20000);
    return pcm_h;
}

// Audio Render Thread Loop
void audioWorker(snd_pcm_t* pcm)
{
    std::vector<int16_t> buffer(NUM_FRAMES * 2);

    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(audioMutex);
            for (size_t f = 0; f < NUM_FRAMES; ++f) {
                float sample = ui.app.isPlaying ? ui.app.engine.sample() : 0.0f;
                int16_t v = (int16_t)(std::clamp(sample, -1.0f, 1.0f) * 32767.0f);
                buffer[f * 2] = v;     // Left channel
                buffer[f * 2 + 1] = v; // Right channel
            }
        }

        if (pcm) {
            int w = snd_pcm_writei(pcm, buffer.data(), NUM_FRAMES);
            if (w < 0) {
                w = snd_pcm_recover(pcm, (int)w, 0);
                if (w < 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}

int main()
{
    std::cout << "Starting zicModWave Desktop PoC (Wavetable Synthesizer Module Simulation)...\n";

    snd_pcm_t* pcm_h = audioInit();
    if (!pcm_h) {
        std::cout << "ALSA audio device not available; continuing in silent audio mode.\n";
    }

    pthread_setname_np(pthread_self(), "zicWave_UI");

    // Spawn high-priority audio thread
    std::thread audioThread(audioWorker, pcm_h);
    pthread_setname_np(audioThread.native_handle(), "zicWave_Audio");
    setThreadRealtime(audioThread.native_handle(), 35, "audio thread");

    // Screen configuration
    Styles appStyles = {
        .screen = { 700, 740 },
        .margin = 0,
        .colors = {
            { 15, 18, 24, 255 },    // background
            { 255, 255, 255, 255 },  // white
            { 255, 255, 255, 255 },  // text
            { 0, 220, 255, 255 },    // primary
            { 0, 220, 140, 255 },    // secondary
            { 40, 45, 55, 255 },     // tertiary
            { 22, 26, 34, 255 }      // quaternary
        }
    };
    auto drawer = std::make_unique<Draw>(appStyles);
    bool needFullRedraw = true;

#ifdef DRAW_SMFL
    runDesktopSFML(*drawer, needFullRedraw, ui);
#endif

    keep_running = false;
    audioThread.join();
    if (pcm_h) {
        snd_pcm_close(pcm_h);
    }

    std::cout << "zicModWave stopped cleanly.\n";
    return 0;
}
