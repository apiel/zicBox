#include <atomic>
#include <iostream>
#include <memory>
#include <pthread.h>
#include <sched.h>
#include <thread>

#include "kickBody.h"
#include "log.h"
#include "mixer.h"
#include "sequencer.h"
#include "ui.h"

std::atomic<bool> keep_running { true };

KickBody kickEngine(44100.0f);
Sequencer seqEngine(44100.0f);
Mixer mixerEngine;
UiPixelDrift* globalUiPtr = nullptr;

#include "audioWorker.h"

#ifdef DRAW_SMFL
#include "runtimeDesktopSFML.h"
#else
#include "runtimeHardware.h"
#endif

namespace {
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

    UiPixelDrift ui(kickEngine, seqEngine, mixerEngine);
    globalUiPtr = &ui;

    std::thread aThread(audioWorker, pcm_h);
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
