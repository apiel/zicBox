#include <alsa/asoundlib.h>
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <sched.h>
#include <thread>
#include <vector>

#include "audio/engines/DriftKick.h"
#include "sequenceBrain.h"
#include "uiPot.h"

// Select desktop SFML vs hardware runtime
#ifdef DRAW_SMFL
#include "runtimeDesktopSFML.h"
#else
#include "runtimeHardware.h"
#endif

// Thread synchronization
std::atomic<bool> keep_running(true);
std::mutex audioMutex;

const unsigned int SAMPLE_RATE = 44100;
const size_t NUM_FRAMES = 256;

// Global state
SequenceBrain brain(SAMPLE_RATE);
DriftKick driftKick(SAMPLE_RATE);
UiPot ui(brain, driftKick);

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
    if (!pcm) return;

    std::vector<int16_t> buffer(NUM_FRAMES * 2);

    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(audioMutex);

            for (size_t f = 0; f < NUM_FRAMES; ++f) {
                // Process clock tick and trigger DriftKick engine on active steps
                brain.processSample(driftKick, nullptr);

                float out = driftKick.sample() * ui.masterVolume;
                int16_t v = (int16_t)(std::clamp(out, -1.0f, 1.0f) * 32767.0f);

                buffer[f * 2] = v;     // Left channel
                buffer[f * 2 + 1] = v; // Right channel
            }
        }

        int w = snd_pcm_writei(pcm, buffer.data(), NUM_FRAMES);
        if (w < 0) {
            w = snd_pcm_recover(pcm, (int)w, 0);
            if (w < 0) {
                std::cerr << "ALSA recovery failed: " << snd_strerror((int)w) << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }
}

int main()
{
    std::cout << "Starting zicPot Desktop PoC (DriftKick Engine & MIDI Master Clock Simulation)...\n";

    snd_pcm_t* pcm_h = audioInit();
    if (!pcm_h) {
        std::cerr << "Failed to initialize ALSA audio. Exiting.\n";
        return 1;
    }

    pthread_setname_np(pthread_self(), "zicPot_UI");

    // Spawn high-priority audio thread
    std::thread audioThread(audioWorker, pcm_h);
    pthread_setname_np(audioThread.native_handle(), "zicPot_Audio");
    setThreadRealtime(audioThread.native_handle(), 35, "audio thread");

    // Screen configuration
    Styles appStyles = {
        .screen = { 960, 620 },
        .margin = 2,
        .colors = { { 18, 20, 26, 255 }, { 255, 255, 255, 255 }, { 120, 120, 130, 255 }, { 0, 180, 255, 255 }, { 10, 10, 12, 255 }, { 28, 28, 32, 255 }, { 35, 35, 40, 255 } }
    };
    auto drawer = std::make_unique<Draw>(appStyles);
    bool needFullRedraw = true;

#ifdef DRAW_SMFL
    runDesktopSFML(*drawer, needFullRedraw, ui, brain, driftKick);
#else
    runHardware(*drawer, appStyles, needFullRedraw, ui, brain, driftKick);
#endif

    keep_running = false;
    audioThread.join();
    snd_pcm_close(pcm_h);

    std::cout << "zicPot stopped cleanly.\n";
    return 0;
}
