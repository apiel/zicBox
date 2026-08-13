#include <csignal>
#include <iostream>
#include <memory>
#include <thread>

#include "draw/draw.h"

#include "zicGridV2/audioWorker.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/project.h"
#include "zicGridV2/studio.h"

#ifdef DRAW_SMFL
#include "zicGridV2/runtimeDesktopSFML.h"
#else
#include "zicGridV2/runtimeHardware.h"
#endif

void signal_handler(int signal)
{
    std::cout << "\nCaught signal " << signal << ", shutting down Zic Grid V2..." << std::endl;
    keep_running = false;
}

int main(int argc, char* argv[])
{
    std::signal(SIGINT, signal_handler);

    std::cout << "==============================================" << std::endl;
    std::cout << "        ZIC GRID V2 - AUDIO DEVICE            " << std::endl;
    std::cout << "  48 RGB Pads Matrix + 12 Encoder Layout      " << std::endl;
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

    loadProject();

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

    if (pcm) {
        snd_pcm_close(pcm);
    }

    std::cout << "[System] Zic Grid V2 clean shutdown completed." << std::endl;

    if (system_halt_requested) {
#if defined(IS_RPI)
        std::cout << "Shutting down RPi..." << std::endl;
        int exitCode = std::system("halt");
        (void)exitCode;
#endif
    }
    return 0;
}
