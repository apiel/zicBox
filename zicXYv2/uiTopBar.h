#pragma once

#include "draw/utils/inRect.h"
#include "helpers/enc.h"
#include "zicXYv2/studio.h"

namespace TopBar {

bool needsRedraw = true;
Rect menuBtnRect, bpmRect, masterRect, transportRect;
Rect trackRect[MAX_TRACKS];
static bool showProjectMenu = false;
uint32_t lastBpmTick = 0;
int height = 20;
int textY = 8;
int btnH = 16;

bool draw(Draw& d, bool needFullRedraw)
{
    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;

    int winW = SCREEN_W;

    d.filledRect({ 0, 0 }, { winW, height }, { .color = d.styles.colors.quaternary });

    int currentX = MARGIN;

    masterRect = { { currentX, 4 }, { 16, btnH } };
    d.filledRect(masterRect.position, masterRect.size, { .color = Color { 60, 60, 75 } });
    d.textCentered({ masterRect.position.x + 8, textY }, "M", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
    if (studio.currentView == ViewMaster) d.filledRect(masterRect.position, { masterRect.size.w, 2 }, { .color = { 255, 255, 255 } });

    currentX += masterRect.size.w + 2;

    for (int i = 0; i < MAX_TRACKS; i++) {
        if (studio.tracks[i] == nullptr) break;
        Track& trk = *studio.tracks[i];
        trackRect[i] = { { currentX, 4 }, { 16, btnH } };
        d.filledRect(trackRect[i].position, trackRect[i].size, { .color = { 50, 50, 50 } });
        d.textCentered({ trackRect[i].position.x + 8, textY }, std::to_string(i + 1), 8, { .color = trk.themeColor, .font = &PoppinsLight_8 });
        if (studio.selTrack == i && studio.currentView == ViewTrack) d.filledRect(trackRect[i].position, { trackRect[i].size.w, 1 }, { .color = trk.themeColor });
        currentX += trackRect[i].size.w + 2;
    }

    transportRect = { { winW - 105, 4 }, { 40, btnH } };
    d.filledRect(transportRect.position, transportRect.size, { .color = studio.isPlaying ? Color { 200, 50, 50 } : Color { 50, 200, 50 } });
    d.textCentered({ transportRect.position.x + 20, textY }, studio.isPlaying ? "STOP" : "PLAY", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    // BPM
    std::stringstream bss;
    bss << "BPM: " << std::fixed << std::setprecision(1) << studio.bpm.load();
    bpmRect = { { winW - 75, 0 }, { 70, height } };
    d.textRight({ winW - 10, textY }, bss.str(), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    return true;
}

void mouseButtonPressed(Point position, bool& needFullRedraw)
{
    if (inRect(menuBtnRect, position)) {
        studio.currentView = ViewMenu;
        needsRedraw = true;
        needFullRedraw = true;
    }

    if (inRect(transportRect, position)) {
        studio.isPlaying = !studio.isPlaying;
        needsRedraw = true;
    }

    if (inRect(masterRect, position)) {
        studio.currentView = ViewMaster;
        needsRedraw = true;
        needFullRedraw = true;
    }

    for (int i = 0; i < MAX_TRACKS; i++) {
        if (studio.tracks[i] == nullptr) break;
        if (inRect(trackRect[i], position)) {
            studio.selTrack = i;
            studio.currentView = ViewTrack;
            needsRedraw = true;
            needFullRedraw = true;
        }
    }
}

bool mouseWheelScrolled(Point position, int delta, uint32_t now, bool shifted)
{
    if (inRect(bpmRect, position)) {
        int scaled = encGetScaledDirection(delta, now, lastBpmTick);
        lastBpmTick = now;
        studio.bpm = std::clamp(studio.bpm + (scaled * (shifted ? 5.0f : 0.5f)), 20.0f, 300.0f);
        studio.updateClock();
        needsRedraw = true;
        return true;
    }

    return false;
}

}