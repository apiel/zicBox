#pragma once

#include "draw/utils/inRect.h"
#include "helpers/enc.h"
#include "zicRack/studio.h"

namespace TopBar {

bool needsRedraw = true;
Rect menuBtnRect, bpmRect, masterRect, transportRect;
Rect trackRect[MAX_TRACKS];
static bool showProjectMenu = false;
uint32_t lastBpmTick = 0;
int height = 35;
int textY = 12;
int btnH = 27;

bool draw(Draw& d, const int winW, bool needFullRedraw)
{
    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;

    d.filledRect({ 0, 0 }, { winW, height }, { .color = d.styles.colors.quaternary });

    // Menu Toggle Button "..."
    menuBtnRect = { { MARGIN, 4 }, { 30, btnH } };
    d.filledRect(menuBtnRect.position, menuBtnRect.size, { .color = showProjectMenu ? Color { 100, 100, 120 } : Color { 60, 60, 75 } });
    d.textCentered({ menuBtnRect.position.x + 15, textY }, "...", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    int currentX = MARGIN + 35;

    masterRect = { { currentX, 4 }, { 60, btnH } };
    d.filledRect(masterRect.position, masterRect.size, { .color = Color { 150, 150, 150 } });
    d.textCentered({ masterRect.position.x + 30, textY }, "MASTER", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    currentX += 68;

    for (int i = 0; i < MAX_TRACKS; i++) {
        if (studio.tracks[i] == nullptr) break;
        Track& trk = *studio.tracks[i];
        trackRect[i] = { { currentX, 4 }, { 60, btnH } };
        d.filledRect(trackRect[i].position, trackRect[i].size, { .color = { 50, 50, 50 } });
        d.textCentered({ trackRect[i].position.x + 30, textY }, trk.engine->getName(), 8, { .color = trk.themeColor, .font = &PoppinsLight_8 });
        currentX += 68;
    }

    // This should rendered in the bottom with the message...
    // // studio.projectPath
    // currentX += 90;
    // d.text({ currentX, textY }, studio.projectPath, 8, { .color = { 150, 150, 150 }, .font = &PoppinsLight_8 });

    transportRect = { { winW - 100, 4 }, { 40, btnH } };
    d.filledRect(transportRect.position, transportRect.size, { .color = studio.isPlaying ? Color { 200, 50, 50 } : Color { 50, 200, 50 } });
    d.textCentered({ transportRect.position.x + 20, textY }, studio.isPlaying ? "STOP" : "PLAY", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    // BPM
    std::stringstream bss;
    bss << "BPM: " << std::fixed << std::setprecision(1) << studio.bpm.load();
    bpmRect = { { winW - 60, 0 }, { 90, height } };
    d.textRight({ winW - MARGIN, textY }, bss.str(), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    return true;
}

void mouseButtonPressed(Point position)
{
    if (inRect(menuBtnRect, position)) {
        showProjectMenu = !showProjectMenu;
        needsRedraw = true;
    }

    if (inRect(transportRect, position)) {
        studio.isPlaying = !studio.isPlaying;
        needsRedraw = true;
    }
}

bool mouseWheelScrolled(Point position, int delta, uint32_t now)
{
    if (inRect(bpmRect, position)) {
        int scaled = encGetScaledDirection(delta, now, lastBpmTick);
        lastBpmTick = now;
        studio.bpm = std::clamp(studio.bpm + (scaled * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.0f : 0.5f)), 20.0f, 300.0f);
        studio.updateClock();
        needsRedraw = true;
        return true;
    }

    return false;
}

}