#include "draw/draw.h"
#include "zicTeK/audioWorker.h"
#include "zicTeK/runtimeDesktopSFML.h"
#include "zicTeK/studio.h"
#include "zicTeK/ui.h"

#include <atomic>
#include <iostream>
#include <memory>
#include <signal.h>
#include <thread>

StudioState studio;
UiZicTeK uiZicTeK;
std::atomic<bool> keep_running { true };

void signalHandler(int signum)
{
    std::cout << "\nInterrupt signal (" << signum << ") received. Exiting...\n";
    keep_running = false;
}

int main(int argc, char* argv[])
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    std::cout << "Starting zicTeK Desktop Application...\n";

    // Setup Default UI Style
    Styles styles = {
        .screen = { 1024, 720 },
        .margin = 0,
        .font = { nullptr, nullptr, nullptr },
        .colors = {
            .background = { 12, 14, 20, 255 },
            .white = { 255, 255, 255, 255 },
            .text = { 220, 230, 245, 255 },
            .primary = { 0, 220, 255, 255 },
            .secondary = { 255, 180, 50, 255 },
            .tertiary = { 255, 90, 120, 255 },
            .quaternary = { 0, 200, 150, 255 } }
    };

    // Allocate Draw instance on the heap (Draw contains 67MB screenBuffer)
    auto drawer = std::make_unique<Draw>(styles);

    // Initialize ALSA audio
    snd_pcm_t* pcm = audioInit(44100);
    if (!pcm) {
        std::cerr << "Warning: ALSA audio device opening failed. Audio synthesis may be disabled.\n";
    }

    // Start Audio Thread
    std::thread audioThread(audioWorker, pcm);

    // Run Desktop SFML Main Loop
    runDesktopSFML(*drawer);

    // Cleanup and exit
    keep_running = false;
    if (audioThread.joinable()) {
        audioThread.join();
    }

    if (pcm) {
        snd_pcm_close(pcm);
    }

    std::cout << "zicTeK exited cleanly.\n";
    return 0;
}
