#include <csignal>
#include <iostream>
#include <memory>
#include <pthread.h>
#include <thread>

#ifndef AUDIO_FOLDER
#define AUDIO_FOLDER std::string("../data/audio")
#endif

#include "audioWorker.h"
#include "draw/draw.h"
#include "studio.h"
#include "ui.h"

#ifdef DRAW_SMFL
#include "runtimeDesktopSFML.h"
#else
#include "runtimeHardware.h"
#endif

StudioState studio;
std::atomic<bool> keep_running { true };

void signalHandler(int signum)
{
    std::cout << "\nInterrupt signal (" << signum << ") received. Exiting zicPixMini...\n";
    keep_running = false;
}

int main(int argc, char* argv[])
{
    pthread_setname_np(pthread_self(), "zicPixMini_UI");
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << "Starting zicPixMini Groovebox Module..." << std::endl;

    snd_pcm_t* pcm = audioInit(44100);
    if (!pcm) {
        std::cerr << "Note: Unable to initialize ALSA audio (running silent UI)." << std::endl;
    }

    std::thread audioThread;
    if (pcm) {
        audioThread = std::thread(audioWorker, pcm);
        pthread_setname_np(audioThread.native_handle(), "zicPixMini_Audio");
    }

    Styles appStyles = {
        .screen = { 240, 320 },
        .margin = 2,
        .colors = { { 15, 15, 18 }, { 255, 255, 255 }, { 120, 120, 130 }, { 0, 180, 255 }, { 10, 10, 12 }, { 28, 28, 32 }, { 35, 35, 40 } }
    };
    auto drawer = std::make_unique<Draw>(appStyles);

    UiPixMini ui;
    bool needFullRedraw = true;

#ifdef DRAW_SMFL
    runDesktopSFML(*drawer, ui, needFullRedraw);
#else
    runHardware(*drawer, ui, needFullRedraw);
#endif

    keep_running = false;
    if (audioThread.joinable()) {
        audioThread.join();
    }

    if (pcm) {
        snd_pcm_close(pcm);
    }

    std::cout << "zicPixMini shutdown complete." << std::endl;
    return 0;
}
