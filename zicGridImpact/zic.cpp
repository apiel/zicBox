#include <csignal>
#include <iostream>
#include <memory>
#include <thread>

#ifdef IS_RPI
#define AUDIO_FOLDER std::string("data/audio")
#else
#define AUDIO_FOLDER std::string("../data/audio")
#endif

#include "draw/draw.h"

#include "zicGridImpact/audioWorker.h"
#include "zicGridImpact/gridState.h"
#include "zicGridImpact/studio.h"

#ifdef DRAW_SMFL
#include "zicGridImpact/runtimeDesktopSFML.h"
#else
#include "zicGridImpact/runtimeHardware.h"
#endif

void signal_handler(int signal)
{
    std::cout << "\nCaught signal " << signal << ", shutting down Zic Grid Impact..." << std::endl;
    keep_running = false;
}

int main(int argc, char* argv[])
{
    std::signal(SIGINT, signal_handler);

    std::cout << "==============================================" << std::endl;
    std::cout << "        ZIC GRID IMPACT - TEKNO GROOVE        " << std::endl;
    std::cout << "  48 NeoTrellis Pads + 12 Encoder Layout      " << std::endl;
    std::cout << "==============================================" << std::endl;

    Styles appStyles = {
        .screen = { SCREEN_W, SCREEN_H },
        .margin = 2,
        .colors = { { 15, 15, 18 }, { 255, 255, 255 }, { 120, 120, 130 }, { 0, 180, 255 }, { 10, 10, 12 }, { 28, 28, 32 }, { 35, 35, 40 } }
    };
    auto drawer = std::make_unique<Draw>(appStyles);

    snd_pcm_t* pcm = audioInit();
    std::thread audioThread;
    if (pcm) {
        audioThread = std::thread(audioWorker, pcm);
        std::cout << "[Audio Engine] Audio thread started on ALSA stream." << std::endl;
    } else {
        std::cout << "[Audio Engine] Running without ALSA stream output." << std::endl;
    }

    bool needFullRedraw = true;

#ifdef DRAW_SMFL
    std::cout << "[Runtime] Starting Desktop SFML UI Runtime..." << std::endl;
    runDesktopSFML(*drawer, needFullRedraw);
#else
    std::cout << "[Runtime] Starting Raspberry Pi Embedded Hardware Runtime..." << std::endl;
    runHardware(*drawer, appStyles, needFullRedraw);
#endif

    keep_running = false;
    if (audioThread.joinable()) {
        audioThread.join();
    }

    if (pcm) snd_pcm_close(pcm);

    std::cout << "[System] Zic Grid Impact clean shutdown completed." << std::endl;

    return 0;
}
