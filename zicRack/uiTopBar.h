#pragma once

#include "zicRack/studio.h"

static sf::IntRect menuBtnRect, bpmRect, transportRect;
static bool showProjectMenu = false;
uint32_t lastBpmTick = 0;

void drawTopBarUI(Draw& d, sf::Vector2u size)
{
    const int winW = (int)size.x;
    d.filledRect({ 0, 0 }, { winW, 25 }, { .color = d.styles.colors.quaternary });

    // Menu Toggle Button "..."
    menuBtnRect = { MARGIN, 4, 30, 17 };
    d.filledRect({ menuBtnRect.left, 4 }, { 30, 17 }, { .color = showProjectMenu ? Color { 100, 100, 120 } : Color { 60, 60, 75 } });
    d.textCentered({ menuBtnRect.left + 15, 7 }, "...", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    int currentX = MARGIN + 35;

    transportRect = { currentX, 4, 60, 17 };
    d.filledRect({ currentX, 4 }, { 60, 17 }, { .color = studio.isPlaying ? Color { 200, 50, 50 } : Color { 50, 200, 50 } });
    d.textCentered({ currentX + 30, 7 }, studio.isPlaying ? "STOP" : "PLAY", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    // studio.projectPath
    currentX += 90;
    d.text({ currentX, 7 }, studio.projectPath, 8, { .color = { 150, 150, 150 }, .font = &PoppinsLight_8 });

    // BPM
    std::stringstream bss;
    bss << "BPM: " << std::fixed << std::setprecision(1) << studio.bpm.load();
    bpmRect = { winW - 100, 0, 90, 25 };
    d.textRight({ winW - MARGIN, 6 }, bss.str(), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
}

void topBarMouseButtonPressed(sf::Event& event, bool& static_needs_redraw)
{
    int mx = event.mouseButton.x, my = event.mouseButton.y;

    if (menuBtnRect.contains(mx, my)) {
        showProjectMenu = !showProjectMenu;
        static_needs_redraw = true;
    }

    if (transportRect.contains(mx, my)) {
        studio.isPlaying = !studio.isPlaying;
        static_needs_redraw = true;
    }
}

bool topBarMouseWheelScrolled(sf::Event& event, bool& static_needs_redraw, uint32_t now)
{
    int mx = event.mouseWheelScroll.x, my = event.mouseWheelScroll.y;
    float delta = event.mouseWheelScroll.delta;

    if (bpmRect.contains(mx, my)) {
        int scaled = encGetScaledDirection(delta, now, lastBpmTick);
        lastBpmTick = now;
        studio.bpm = std::clamp(studio.bpm + (scaled * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.0f : 0.5f)), 20.0f, 300.0f);
        studio.updateClock();
        static_needs_redraw = true;
        return true;
    }

    return false;
}