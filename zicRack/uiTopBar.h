#pragma once

#include "draw/utils/inRect.h"
#include "zicRack/studio.h"

namespace TopBar {

bool needsRedraw = true;
Rect menuBtnRect, bpmRect, transportRect;
static bool showProjectMenu = false;
uint32_t lastBpmTick = 0;
int height = 25;

int draw(Draw& d, const int winW, bool needFullRedraw)
{
    if (!needsRedraw && !needFullRedraw) return height;
    needsRedraw = false;

    d.filledRect({ 0, 0 }, { winW, height }, { .color = d.styles.colors.quaternary });

    // Menu Toggle Button "..."
    menuBtnRect = { { MARGIN, 4 }, { 30, 17 } };
    d.filledRect({ menuBtnRect.position.x, 4 }, { 30, 17 }, { .color = showProjectMenu ? Color { 100, 100, 120 } : Color { 60, 60, 75 } });
    d.textCentered({ menuBtnRect.position.x + 15, 7 }, "...", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    int currentX = MARGIN + 35;

    transportRect = { { currentX, 4 }, { 60, 17 } };
    d.filledRect({ currentX, 4 }, { 60, 17 }, { .color = studio.isPlaying ? Color { 200, 50, 50 } : Color { 50, 200, 50 } });
    d.textCentered({ currentX + 30, 7 }, studio.isPlaying ? "STOP" : "PLAY", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    // studio.projectPath
    currentX += 90;
    d.text({ currentX, 7 }, studio.projectPath, 8, { .color = { 150, 150, 150 }, .font = &PoppinsLight_8 });

    // BPM
    std::stringstream bss;
    bss << "BPM: " << std::fixed << std::setprecision(1) << studio.bpm.load();
    bpmRect = { { winW - 100, 0 }, { 90, height } };
    d.textRight({ winW - MARGIN, 6 }, bss.str(), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    return height;
}

void mouseButtonPressed(Point position, bool& needsGlobalRedraw)
{
    if (inRect(menuBtnRect, position)) {
        showProjectMenu = !showProjectMenu;
        needsGlobalRedraw = true;
        needsRedraw = true;
    }

    if (inRect(transportRect, position)) {
        studio.isPlaying = !studio.isPlaying;
        needsGlobalRedraw = true;
        needsRedraw = true;
    }
}

bool mouseWheelScrolled(Point position, bool& needsGlobalRedraw, int delta, uint32_t now)
{
    // float delta = event.mouseWheelScroll.delta;

    if (inRect(bpmRect, position)) {
        int scaled = encGetScaledDirection(delta, now, lastBpmTick);
        lastBpmTick = now;
        studio.bpm = std::clamp(studio.bpm + (scaled * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.0f : 0.5f)), 20.0f, 300.0f);
        studio.updateClock();
        needsGlobalRedraw = true;
        needsRedraw = true;
        return true;
    }

    return false;
}

}