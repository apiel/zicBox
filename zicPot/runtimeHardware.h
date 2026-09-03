#pragma once

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

#include "audio/engines/DriftKick.h"
#include "draw/draw.h"
#include "sequenceBrain.h"
#include "uiPot.h"

extern std::atomic<bool> keep_running;
extern std::mutex audioMutex;

inline void runHardware(Draw& d, const Styles& appStyles, bool& needFullRedraw, UiPot& ui, SequenceBrain& brain, DriftKick& kick)
{
    std::cout << "Starting embedded hardware runtime stub for zicPot...\n";
    while (keep_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
