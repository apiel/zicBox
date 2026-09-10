#pragma once

#include "draw/draw.h"
#include "log.h"
#include "ui.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#ifdef DRAW_ST7789
#include "hardware/display/ST7789.h"
#endif

extern std::atomic<bool> keep_running;

void runHardware(Draw& d, UiKick& ui, bool& needFullRedraw)
{
    const int SCREEN_W = 640;
    const int SCREEN_H = 360;
    d.setScreenSize({ SCREEN_W, SCREEN_H });

    logInfo("Starting zicKick Embedded Hardware Runtime loop...");

    while (keep_running) {
        if (ui.drawUI(d, SCREEN_W, SCREEN_H, needFullRedraw)) {
#ifdef DRAW_ST7789
            DrawToST7789(d.screenBuffer, SCREEN_W, SCREEN_H);
#endif
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}
