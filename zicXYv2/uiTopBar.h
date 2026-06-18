#pragma once

#include "draw/utils/Icon.h"
#include "draw/utils/inRect.h"
#include "helpers/enc.h"
#include "zicXYv2/studio.h"
#include "zicXYv2/uiMessage.h"
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

void drawSideInfo(Draw& d, int y, int winW, Icon& icon)
{
    masterRect = { { winW - 95, y }, { 16, btnH } };
    d.filledRect(masterRect.position, masterRect.size, { .color = Color { 60, 60, 75 } });
    d.textCentered({ masterRect.position.x + 8, masterRect.position.y + 4 }, "M", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
    if (studio.currentView == ViewMaster) d.filledRect(masterRect.position, { masterRect.size.w, 2 }, { .color = { 255, 255, 255 } });

    transportRect = { { winW - 80, y + 2 }, { 20, btnH - 4 } };
    if (studio.isPlaying) icon.play({ transportRect.position.x + 10, transportRect.position.y + 2 }, { 8, 8 }, { 255, 255, 255 }, true);
    else icon.stop({ transportRect.position.x + 10, transportRect.position.y + 2 }, { 8, 8 }, { 255, 255, 255 }, true);

    // BPM
    std::stringstream bss;
    bss << "BPM: " << std::fixed << std::setprecision(1) << studio.bpm.load();
    bpmRect = { { winW - 70, 0 }, { 70, height } };
    d.textRight({ winW - 10, y + 4 }, bss.str(), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
}

void drawButtonArray(Draw& d, int y, int btnW, int halfBtnW, Icon& icon, const std::vector<std::string>& keys, int pressedKey = -1)
{
    int currentX = 0;
    for (int i = 0; i < keys.size(); i++) {
        const std::string& key = keys[i];
        menuBtnRect = { { currentX, y }, { btnW, btnH } };
        d.filledRect(menuBtnRect.position, menuBtnRect.size, { .color = pressedKey == i ? Color { 40, 40, 40 } : Color { 50, 50, 50 } });
        if (key[0] == '&') {
            icon.render(key, { menuBtnRect.position.x + halfBtnW, menuBtnRect.position.y + 4 }, { 8, 8 }, pressedKey == i ? Color { 150, 150, 150 } : Color { 255, 255, 255 });
        } else {
            d.textCentered({ menuBtnRect.position.x + halfBtnW, menuBtnRect.position.y + 4 }, key, 8, { .color = pressedKey == i ? Color { 150, 150, 150 } : Color { 255, 255, 255 }, .font = &PoppinsLight_8 });
        }
        currentX += menuBtnRect.size.w + 2;
    }
}

bool draw(Draw& d, const int winW, bool needFullRedraw, int& currentY)
{
    int y = currentY;
    currentY += height;
    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;

    d.filledRect({ 0, 0 }, { winW, height }, { .color = d.styles.colors.quaternary });

    Icon icon(d);
    drawSideInfo(d, y, winW, icon);

    int btnW = (winW / 8) - 2;
    int halfBtnW = btnW / 2;

    if (studio.currentCombinationKey == KeyProject) {
        drawButtonArray(d, y, btnW, halfBtnW, icon, { "Reload", "Load", "SaveAs", "Save", "Project" }, 4);
        y += btnH + 2;
        drawButtonArray(d, y, btnW, halfBtnW, icon, { "---", "---", "---", "---", "---", "---", "---", "---" });
    } else if (studio.currentCombinationKey == KeyMute) {
        drawButtonArray(d, y, btnW, halfBtnW, icon, { "---", "---", "Stop", "Mute", studio.isPlaying ? "Pause" : "Play" }, 3);
        y += btnH + 2;
        drawTracks(d, y, btnW, halfBtnW, icon);
    } else if (studio.currentCombinationKey == KeyView) {
        drawButtonArray(d, y, btnW, halfBtnW, icon, { "View", "Master", "Seq", "Clips", "---" }, 0);
        y += btnH + 2;
        drawTracks(d, y, btnW, halfBtnW, icon);
    } else if (studio.currentView == ViewTrack || studio.currentView == ViewMaster) {
        drawButtonArray(d, y, btnW, halfBtnW, icon, { "View", "&icon::arrowDown::filled", "&icon::arrowUp::filled", "Mute", "Project" });
        y += btnH + 2;
        drawTracks(d, y, btnW, halfBtnW, icon);
    } else if (studio.currentView == ViewSeq) {
        drawButtonArray(d, y, btnW, halfBtnW, icon, { "View", "&icon::arrowUp::filled", "Toggle", "Mute", "Project" });
        y += btnH + 2;
        drawButtonArray(d, y, btnW, halfBtnW, icon, { "&icon::arrowLeft::filled", "&icon::arrowDown::filled", "&icon::arrowRight::filled", "---", "---", "---", "---", "---" });
    }

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

void keyPressed(int key, bool& needFullRedraw)
{
    if (studio.currentCombinationKey == KeyView) {
        if (key == KEY_F2) {
            studio.currentView = ViewMaster;
            needsRedraw = true;
            needFullRedraw = true;
        } else if (key == KEY_F3) {
            studio.currentView = ViewSeq;
            needsRedraw = true;
            needFullRedraw = true;
        } else if (key == KEY_F4) {
            studio.currentView = ViewClips;
            needsRedraw = true;
            needFullRedraw = true;
        } else if (key >= KEY_1 && key <= KEY_8) {
            studio.selTrack = key - KEY_1;
            studio.currentView = ViewTrack;
            needsRedraw = true;
            needFullRedraw = true;
        }
    } else if (studio.currentCombinationKey == KeyMute) {
        if (key == KEY_F3) {
            // HERE should reset sequencer
            studio.isPlaying = false;
            needsRedraw = true;
        } else if (key == KEY_F5) {
            studio.isPlaying = !studio.isPlaying;
            needsRedraw = true;
        } else if (key >= KEY_1 && key <= KEY_8) {
            int trkIdx = key - KEY_1;
            studio.tracks[trkIdx]->isMuted = !studio.tracks[trkIdx]->isMuted;
            needsRedraw = true;
        }
    } else if (studio.currentCombinationKey == KeyProject) {
        if (key == KEY_F1) {
            if (loadProject()) {
                UiMessage::show("Project loaded", needFullRedraw);
                needFullRedraw = true;
            }
        } else if (key == KEY_F4) {
            if (saveProject()) {
                UiMessage::show("Project saved", needFullRedraw);
                needFullRedraw = true;
            }
        }
    } else if (studio.currentView == ViewTrack || studio.currentView == ViewMaster) {
        if (key == KEY_F1) {
            studio.currentCombinationKey = KeyView;
            needFullRedraw = true;
        } else if (key == KEY_F4) {
            studio.currentCombinationKey = KeyMute;
            needFullRedraw = true;
        } else if (key == KEY_F5) {
            studio.currentCombinationKey = KeyProject;
            needFullRedraw = true;
        }
    }
}

void keyReleased(int key, bool& needFullRedraw)
{
    if (studio.currentView == ViewTrack || studio.currentView == ViewMaster) {
        if (key == KEY_F1 && studio.currentCombinationKey == KeyView) {
            studio.currentCombinationKey = KeyNone;
            needFullRedraw = true;
        } else if (key == KEY_F4 && studio.currentCombinationKey == KeyMute) {
            studio.currentCombinationKey = KeyNone;
            needFullRedraw = true;
        } else if (key == KEY_F5 && studio.currentCombinationKey == KeyProject) {
            studio.currentCombinationKey = KeyNone;
            needFullRedraw = true;
        }
    }
}

}