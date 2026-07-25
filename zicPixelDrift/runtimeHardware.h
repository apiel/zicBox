#pragma once

#include <chrono>
#include <iostream>
#include <thread>

#include "draw/draw.h"
#include "draw/drawRendererST7789.h"
#include "ui.h"

extern std::atomic<bool> keep_running;

void runHardware(Draw& d, UiPixelDrift& ui, bool& needFullRedraw)
{
    const int SCREEN_W = 320;
    const int SCREEN_H = 176;

    d.setScreenSize({ SCREEN_W, SCREEN_H });
    DrawRendererST7789 renderer;

    logInfo("Starting zicPixelDrift Hardware Loop");

    while (keep_running) {
        if (ui.drawUI(d, SCREEN_W, SCREEN_H, needFullRedraw)) {
            renderer.update(d.screenBuffer, SCREEN_W, SCREEN_H);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
}
