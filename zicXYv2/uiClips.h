#pragma once

#include "draw/utils/inRect.h"
#include "helpers/enc.h"
#include "zicXYv2/draw.h"
#include "zicXYv2/project.h"
#include "zicXYv2/studio.h"
#include <algorithm>
#include <chrono>
#include <cstdint>

#include "zicXYv2/uiClipName.h"

namespace UiClips {

bool needsRedraw = true;
int top = 0;
Rect gridRect = { { -1, -1 }, { -1, -1 } };
int selectedClipIdx = 0;
int prevPendingClipIdx[MAX_TRACKS] = { -1 };

bool leftHeld = false;
bool rightHeld = false;
uint64_t leftNextMoveMs = 0;
uint64_t rightNextMoveMs = 0;

bool draw(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY)
{
    using namespace std::chrono;
    static steady_clock::time_point lastBlink = steady_clock::now();
    static bool blinkOn = false;

    const uint64_t initialDelayMs = 400;
    const uint64_t repeatIntervalMs = 80;
    auto nowMs = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();

    if (leftHeld) {
        if (leftNextMoveMs == 0) {
            leftNextMoveMs = nowMs + initialDelayMs;
        } else if (nowMs >= leftNextMoveMs) {
            if (selectedClipIdx > 0) {
                selectedClipIdx--;
                needsRedraw = true;
            }
            leftNextMoveMs = nowMs + repeatIntervalMs;
        }
    }

    if (rightHeld) {
        if (rightNextMoveMs == 0) {
            rightNextMoveMs = nowMs + initialDelayMs;
        } else if (nowMs >= rightNextMoveMs) {
            if (selectedClipIdx < MAX_CLIP_COUNT - 1) {
                selectedClipIdx++;
                needsRedraw = true;
            }
            rightNextMoveMs = nowMs + repeatIntervalMs;
        }
    }

    bool hasPending = false;
    for (int t = 0; t < MAX_TRACKS; t++) {
        if (studio.tracks[t] == nullptr) continue;
        Track& trk = *studio.tracks[t];
        if (prevPendingClipIdx[t] >= 0 && trk.pendingClipIdx == -1) {
            needsRedraw = true;
        }
        prevPendingClipIdx[t] = trk.pendingClipIdx;
        if (trk.pendingClipIdx >= 0) hasPending = true;
    }

    // toggle blink state every 500ms when any pending clip exists
    if (hasPending) {
        auto now = steady_clock::now();
        if (now - lastBlink >= milliseconds(500)) {
            blinkOn = !blinkOn;
            lastBlink = now;
            needsRedraw = true;
        }
    }

    if (!needsRedraw && !needFullRedraw) {
        return false;
    }

    needsRedraw = false;

    top = currentY + 2;

    const int cols = MAX_CLIP_COUNT; // 32
    const int rows = MAX_TRACKS; // 8

    // left column for track names
    const int leftColW = 60;
    int gridW = winW - (MARGIN * 2) - leftColW;
    int rowH = std::max(14, (winH - top - UiDraw::ROW_H - 2) / rows);
    int cellW = std::max(2, gridW / cols);

    gridRect = { { MARGIN + leftColW, top }, { gridW, rowH * rows } };

    // background
    d.filledRect({ MARGIN, top }, { leftColW, rowH * rows }, { .color = d.styles.colors.background });
    d.filledRect(gridRect.position, gridRect.size, { .color = d.styles.colors.quaternary });

    // Draw each track row with 32 clip cells
    for (int t = 0; t < rows; t++) {
        if (studio.tracks[t] == nullptr) break;
        Track& trk = *studio.tracks[t];
        int y = top + t * rowH;

        // left column: track label + mute
        d.filledRect({ MARGIN, y }, { leftColW, rowH }, { .color = { 30, 30, 30 } });
        d.text({ MARGIN + 6, y + 4 }, "Track " + std::to_string(t + 1), 8, { .color = trk.themeColor, .font = &PoppinsLight_8 });
        if (trk.isMuted) {
            Icon icon(d);
            icon.mute({ MARGIN + leftColW - 14, y + 4 }, { 10, 10 }, { 155, 155, 155 }, true);
        }

        for (int c = 0; c < cols; c++) {
            int x = gridRect.position.x + c * cellW;

            Color bg = { 40, 40, 45 };
            Color text = { 180, 180, 190 };

            Clip& clip = trk.clips[c];
            if (trk.activeClipIdx == c) {
                bg = trk.themeColor;
                text = { 255, 255, 255 };
            } else if (clip.saved) {
                bg = trk.themeColor;
                bg.a = 90;
                text = { 210, 210, 220 };
            }

            d.filledRect({ x + 1, y + 1 }, { cellW - 2, rowH - 2 }, { .color = bg });

            if (cellW > 20) {
                d.text({ x + 4, y + 4 }, std::to_string(c + 1), 8, { .color = text, .font = &PoppinsLight_8 });
            }

            if (studio.selTrack == t && selectedClipIdx == c) {
                d.rect({ x, y }, { cellW, rowH }, { .color = { 255, 255, 255 } });
            }

            if (trk.pendingClipIdx == c) {
                Icon icon(d);
                Color pendingColor = blinkOn ? Color { 255, 255, 255 } : trk.themeColor;
                icon.play({ x + cellW / 2 - 2, y + rowH / 2 - 2 }, { 4, 4 }, pendingColor, true);
            }
        }
    }

    Track& trk = *studio.tracks[studio.selTrack];
    Clip& clip = trk.clips[selectedClipIdx];

    int infoY = top + rowH * rows + 4;
    int infoH = winH - infoY - MARGIN;
    if (infoH < 16) infoH = 16;
    int infoW = winW - (MARGIN * 2);
    Rect infoRect = { { MARGIN, infoY }, { infoW, infoH } };

    d.filledRect(infoRect.position, infoRect.size, { .color = d.styles.colors.quaternary });

    int textX = d.text({ MARGIN + 4, infoY + 4 }, "Name: ", 8, { .color = { 185, 185, 185 }, .font = &PoppinsLight_8 });
    d.text({ textX, infoY + 4 }, clip.name, 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    textX = d.text({ MARGIN + 90, infoY + 4 }, "Engine: ", 8, { .color = { 185, 185, 185 }, .font = &PoppinsLight_8 });
    d.text({ textX, infoY + 4 }, engineRegistry[clip.engineId].name, 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    if (clip.saved && clip.sequence.size() > 0) {
        const int previewCols = 16;
        const int previewRows = 4;
        int padding = 4;
        int previewX = infoRect.position.x + padding;
        int previewY = infoRect.position.y + 18;
        int previewW = infoRect.size.w - padding * 2;
        int previewH = infoRect.size.h - 20 - padding;

        int cellW = std::max(1, std::min(previewW / previewCols, 10));
        int cellH = std::max(1, std::min(previewH / previewRows, 8));
        int gridW = cellW * previewCols;
        int gridH = cellH * previewRows;

        if (gridW + padding * 2 <= infoRect.size.w && gridH + 20 + padding <= infoRect.size.h) {
            for (int i = 0; i < previewCols * previewRows; i++) {
                int row = i / previewCols;
                int col = i % previewCols;
                Point cellPos = { previewX + col * cellW, previewY + row * cellH };
                Rect cellRect = { cellPos, { cellW - 1, cellH - 1 } };
                bool active = false;
                if (i < (int)clip.sequence.size()) {
                    active = clip.sequence[i].active;
                }
                if (active) {
                    d.filledRect(cellRect.position, cellRect.size, { .color = trk.themeColor });
                } else {
                    d.rect(cellRect.position, cellRect.size, { .color = { 80, 80, 90 } });
                }
            }
        }
    } else {
        d.text({ MARGIN + 4, infoY + 18 }, "No saved clip preview", 8, { .color = { 180, 180, 190 }, .font = &PoppinsLight_8 });
    }

    return true;
}

void mouseButtonPressed(Point position, const int winW, bool& needFullRedraw)
{
    if (studio.currentView == ViewClipName) {
        UiViewClipName::mouseButtonPressed(position, needFullRedraw);
        return;
    }

    // support clicking the left track column or a clip cell
    const int leftColW = 72;
    if (position.x < MARGIN || position.y < top) return;

    // click on left column => select track
    if (position.x >= MARGIN && position.x < MARGIN + leftColW) {
        int rowH = gridRect.size.h / MAX_TRACKS;
        int tr = (position.y - top) / rowH;
        if (tr >= 0 && tr < MAX_TRACKS && studio.tracks[tr] != nullptr) {
            studio.selTrack = tr;
            studio.currentView = ViewTrack;
            needsRedraw = true;
            needFullRedraw = true;
        }
        return;
    }

    if (!inRect(gridRect, position)) return;

    int cols = MAX_CLIP_COUNT;
    int rowH = gridRect.size.h / MAX_TRACKS;
    int col = (position.x - gridRect.position.x) / (gridRect.size.w / cols);
    int row = (position.y - gridRect.position.y) / rowH;
    if (col < 0 || col >= cols || row < 0 || row >= MAX_TRACKS) return;

    if (studio.tracks[row] == nullptr) return;
    Track& trk = *studio.tracks[row];

    // select that track and clip
    studio.selTrack = row;
    selectedClipIdx = col;
    studio.selStep = -1;
    needsRedraw = true;
    needFullRedraw = true;
}

void keyPressed(int key, bool& needFullRedraw)
{
    if (studio.currentView == ViewClipName) {
        UiViewClipName::keyPressed(key, needFullRedraw);
        return;
    }

    if (studio.currentCombinationKey == KeyView) return;
    if (studio.currentView != ViewClips) return;

    Track& trk = *studio.tracks[studio.selTrack];

    if (key == KEY_1) { // Left
        if (selectedClipIdx > 0) {
            selectedClipIdx--;
            needsRedraw = true;
        }
        if (!leftHeld) {
            leftHeld = true;
            leftNextMoveMs = 0;
        }
    } else if (key == KEY_2) { // Down
        if (studio.selTrack < MAX_TRACKS - 1) {
            studio.selTrack++;
            needsRedraw = true;
        }
    } else if (key == KEY_3) { // Right
        if (selectedClipIdx < MAX_CLIP_COUNT - 1) {
            selectedClipIdx++;
            needsRedraw = true;
        }
        if (!rightHeld) {
            rightHeld = true;
            rightNextMoveMs = 0;
        }
    } else if (key == KEY_F2) { // Up
        if (studio.selTrack > 0) {
            studio.selTrack--;
            needsRedraw = true;
        }
    } else if (key == KEY_F3) { // Shift
        studio.currentCombinationKey = KeyShift;
        needFullRedraw = true;
    } else if (key == KEY_4) {
        if (!trk.clips[selectedClipIdx].saved) {
            saveClip(trk, selectedClipIdx);
            loadClip(trk, selectedClipIdx);
            trk.pendingClipIdx = -1;
        } else {
            if (trk.pendingClipIdx == selectedClipIdx) {
                loadClip(trk, selectedClipIdx);
                trk.pendingClipIdx = -1;
            } else {
                trk.pendingClipIdx = selectedClipIdx;
            }
        }
        needsRedraw = true;
    } else if (key == KEY_5) {
        studio.masterScatter.toggleMode(0);
    } else if (key == KEY_6) {
        studio.masterScatter.toggleMode(1);
    } else if (key == KEY_7) {
        studio.masterScatter.toggleMode(2);
    } else if (key == KEY_8) {
        studio.masterScatter.toggleMode(3);
    } else if (studio.currentCombinationKey == KeyShift) {
        if (key == KEY_8) { // Delete
            trk.clips[selectedClipIdx].saved = false;
            needsRedraw = true;
        } else if (key == KEY_F1) { // Rename
            UiViewClipName::newClipName = trk.clips[selectedClipIdx].name;
            UiKeyboard::keyboardSelectedRow = 0;
            UiKeyboard::keyboardSelectedCol = 0;
            UiViewClipName::needsRedraw = true;
            studio.currentView = ViewClipName;
            studio.currentCombinationKey = KeyNone;
            needFullRedraw = true;
        } else if (key == KEY_F4) { // Copy
            // To implement
        } else if (key == KEY_F5) { // Paste
            // To implement
        }
    }
}

void keyReleased(int key, bool& needFullRedraw)
{
    if (studio.currentView != ViewClips) return;

    if (key == KEY_1) {
        leftHeld = false;
        leftNextMoveMs = 0;
    } else if (key == KEY_3) {
        rightHeld = false;
        rightNextMoveMs = 0;
    } else if (key == KEY_F3) {
        studio.currentCombinationKey = KeyNone;
        needFullRedraw = true;
    } else if (key == KEY_5) {
        studio.masterScatter.toggleMode(0);
    } else if (key == KEY_6) {
        studio.masterScatter.toggleMode(1);
    } else if (key == KEY_7) {
        studio.masterScatter.toggleMode(2);
    } else if (key == KEY_8) {
        studio.masterScatter.toggleMode(3);
    }
}

// FIXME this need to be removed as it is not used...
// TODO somehow to have the engine type here is not ideal
// because we cannot preview the sound
// also for sample engine, we should keep the same sample between engine
bool mouseWheelScrolled(Point position, int delta, const int winW, uint32_t now, bool shifted)
{
    if (studio.currentView != ViewClips) return false;
    if (studio.selTrack < 0 || studio.selTrack >= MAX_TRACKS) return false;
    if (studio.tracks[studio.selTrack] == nullptr) return false;

    Track& trk = *studio.tracks[studio.selTrack];
    Clip& clip = trk.clips[selectedClipIdx];
    if (!clip.saved) return false;

    const int paramsPerRow = 4;
    const int maxVisibleRows = 1;
    const int paramsTopY = top + (gridRect.size.h / MAX_TRACKS) * MAX_TRACKS + 4;
    int usableWidth = winW - (MARGIN * 2);
    int adjustedColW = usableWidth / paramsPerRow;

    int visualRow = (position.y - paramsTopY) / UiDraw::ROW_H;
    int col = (position.x - MARGIN) / adjustedColW;
    if (visualRow < 0 || visualRow >= maxVisibleRows || col < 0 || col >= paramsPerRow) {
        return false;
    }

    int finalPIdx = col;
    if (finalPIdx != 0) return false; // only the engine selector is exposed for clips today

    int scaled = encGetScaledDirection(delta, now, trk.lastShiftTicks[finalPIdx]);
    trk.lastShiftTicks[finalPIdx] = now;

    int currentEngineIdx = clip.engineId;
    currentEngineIdx += scaled * (shifted ? 5 : 1);
    currentEngineIdx = std::clamp(currentEngineIdx, 0, ENGINE_REGISTRY_COUNT - 1);

    if (currentEngineIdx != clip.engineId) {
        clip.engineId = (uint8_t)currentEngineIdx;
        clip.validated = false;

        if (trk.activeClipIdx == selectedClipIdx) {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            trk.setEngine(currentEngineIdx);
        }
        needsRedraw = true;
    }

    return true;
}
}