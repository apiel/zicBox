#pragma once

#include "draw/utils/inRect.h"
#include "zicXYv2/draw.h"
#include "zicXYv2/project.h"
#include "zicXYv2/studio.h"

namespace UiClips {

bool needsRedraw = true;
int top = 0;
Rect gridRect = { { -1, -1 }, { -1, -1 } };
int selectedClipIdx = 0;
int prevPendingClipIdx[MAX_TRACKS] = { -1 };

bool draw(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY)
{
    for (int t = 0; t < MAX_TRACKS; t++) {
        if (studio.tracks[t] == nullptr) continue;
        Track& trk = *studio.tracks[t];
        if (prevPendingClipIdx[t] >= 0 && trk.pendingClipIdx == -1) {
            needsRedraw = true;
        }
        prevPendingClipIdx[t] = trk.pendingClipIdx;
    }

    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;

    top = currentY + 2;

    const int cols = MAX_CLIP_COUNT; // 32
    const int rows = MAX_TRACKS; // 8

    // left column for track names
    const int leftColW = 60;
    int gridW = winW - (MARGIN * 2) - leftColW;
    int rowH = std::max(14, (winH - top - 10) / rows);
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
                // d.filledCircle({ x + cellW / 2, y + rowH / 2 }, 2, { .color = { 255, 255, 255 } });
                Icon icon(d);
                icon.play({ x + cellW / 2 - 2, y + rowH / 2 - 2 }, { 4, 4 }, { 255, 255, 255 }, true);
            }
        }
    }

    return true;
}

void mouseButtonPressed(Point position, const int winW, bool& needFullRedraw)
{
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
    if (studio.currentCombinationKey == KeyView) return;
    if (studio.currentView != ViewClips) return;

    Track& trk = *studio.tracks[studio.selTrack];

    if (key == KEY_1) { // Left
        if (selectedClipIdx > 0) {
            selectedClipIdx--;
            needsRedraw = true;
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
    } else if (key == KEY_F2) { // Up
        if (studio.selTrack > 0) {
            studio.selTrack--;
            needsRedraw = true;
        }
    } else if (key == KEY_4) { // Activate now
        if (!trk.clips[selectedClipIdx].saved) {
            saveClip(trk, selectedClipIdx);
        }
        loadClip(trk, selectedClipIdx);
        trk.pendingClipIdx = -1;
        needsRedraw = true;
    } else if (key == KEY_5) { // Activate at next loop
        trk.pendingClipIdx = selectedClipIdx;
        needsRedraw = true;
    } else if (key == KEY_8) { // Delete
        trk.clips[selectedClipIdx].saved = false;
        needsRedraw = true;
    }
}

void keyReleased(int key, bool& needFullRedraw)
{
}

bool mouseWheelScrolled(Point position, int delta, const int winW, uint32_t now, bool shifted)
{
    // no special wheel behaviour for now
    return false;
}
}
