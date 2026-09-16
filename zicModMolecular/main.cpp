#include <atomic>
#include <csignal>
#include <iostream>

#include "displayView.h"
#include "draw/draw.h"
#include "runtimeDesktopSFML.h"
#include "zicApp.h"

static std::atomic<bool> keepRunning(true);

void signalHandler(int sig)
{
    std::cout << "\nSignal " << sig << " received, stopping..." << std::endl;
    keepRunning = false;
}

int main()
{
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << "Starting zicModMolecular - 480x800 Portrait Touch Synthesizer..." << std::endl;

    ZicApp app;
    DisplayView displayView;

    Styles appStyles = {
        { DisplayView::SCREEN_W, DisplayView::SCREEN_H },
        2,
        { nullptr, nullptr, nullptr },
        {
            { 15, 18, 24, 255 },
            { 255, 255, 255, 255 },
            { 120, 120, 130, 255 },
            { 0, 210, 255, 255 },
            { 10, 10, 12, 255 },
            { 28, 28, 32, 255 },
            { 35, 35, 40, 255 }
        }
    };

    Draw drawer(appStyles);

    runDesktopSFML(drawer, app, displayView, keepRunning);

    std::cout << "zicModMolecular terminated cleanly." << std::endl;
    return 0;
}
