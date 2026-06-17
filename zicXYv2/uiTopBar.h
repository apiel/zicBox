#pragma once

#include "draw/utils/inRect.h"
#include "helpers/enc.h"
#include "zicXYv2/studio.h"
#include "draw/utils/Icon.h"
namespace TopBar {

bool needsRedraw = true;
Rect menuBtnRect, bpmRect, masterRect, transportRect;
Rect trackRect[MAX_TRACKS];
static bool showProjectMenu = false;
uint32_t lastBpmTick = 0;
int btnH = 16;
int height = btnH * 2 + 2;

void drawTracks(Draw& d, int y, int btnW, int halfBtnW, Icon& icon)
{
    int currentX = 0;

    for (int i = 0; i < MAX_TRACKS; i++) {
        if (studio.tracks[i] == nullptr) break;
        Track& trk = *studio.tracks[i];
        trackRect[i] = { { currentX, y }, { btnW, btnH } };
        d.filledRect(trackRect[i].position, trackRect[i].size, { .color = { 50, 50, 50 } });
        d.textCentered({ trackRect[i].position.x + halfBtnW, trackRect[i].position.y + 4 }, std::to_string(i + 1), 8, { .color = trk.themeColor, .font = &PoppinsLight_8 });
        if (studio.selTrack == i && studio.currentView == ViewTrack) d.filledRect(trackRect[i].position, { trackRect[i].size.w, 1 }, { .color = trk.themeColor });
        if (trk.isMuted) {
            icon.mute({ trackRect[i].position.x + halfBtnW + 7, trackRect[i].position.y + 7 }, { 8, 8 }, { 155, 155, 155 }, true);
        }
        currentX += trackRect[i].size.w + 2;
    }

}

bool draw(Draw& d, const int winW, bool needFullRedraw, int& currentY)
{
    int y = currentY;
    currentY += height;
    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;

    d.filledRect({ 0, 0 }, { winW, height }, { .color = d.styles.colors.quaternary });

    int btnW = (winW / 8) - 2;
    int halfBtnW = btnW / 2;
    int currentX = 0;

    // std::string keys[] = { "File", "Edit", "View", "Tools", "Help" };
    // Reload Load  SaveAs Save   Project
    std::string keys[] = { "Reload", "Load", "Save", "SaveAs", "Project" };

    for (auto key : keys) {
        menuBtnRect = { { currentX, y }, { btnW, btnH } };
        d.filledRect(menuBtnRect.position, menuBtnRect.size, { .color = { 50, 50, 50 } });
        d.textCentered({ menuBtnRect.position.x + halfBtnW, menuBtnRect.position.y + 4 }, key, 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
        currentX += menuBtnRect.size.w + 2;
    }

    masterRect = { { winW - 95, y }, { 16, btnH } };
    d.filledRect(masterRect.position, masterRect.size, { .color = Color { 60, 60, 75 } });
    d.textCentered({ masterRect.position.x + 8, masterRect.position.y + 4 }, "M", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
    if (studio.currentView == ViewMaster) d.filledRect(masterRect.position, { masterRect.size.w, 2 }, { .color = { 255, 255, 255 } });

    transportRect = { { winW - 80, y + 2 }, { 20, btnH - 4 } };
    Icon icon(d);
    if (studio.isPlaying) icon.play({ transportRect.position.x + 10, transportRect.position.y + 2 }, { 8, 8 }, { 255, 255, 255 }, true);
    else icon.stop({ transportRect.position.x + 10, transportRect.position.y + 2 }, { 8, 8 }, { 255, 255, 255 }, true);

    // BPM
    std::stringstream bss;
    bss << "BPM: " << std::fixed << std::setprecision(1) << studio.bpm.load();
    bpmRect = { { winW - 70, 0 }, { 70, height } };
    d.textRight({ winW - 10, y + 4 }, bss.str(), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    y += btnH + 2;
    drawTracks(d, y, btnW, halfBtnW, icon);

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