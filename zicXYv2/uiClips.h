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
int prevPendingClipIdx[MAX_TRACKS] = { -1, -1, -1, -1, -1, -1, -1, -1 };
int prevChainActiveIdx[MAX_TRACKS] = { -1, -1, -1, -1, -1, -1, -1, -1 };
Clip copiedClip;
bool hasCopiedClip = false;

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

    Track& selTrk = *studio.tracks[studio.selTrack];
    if (selTrk.chainPlaying && studio.isPlaying) {
        if (selTrk.chainActiveIdx != prevChainActiveIdx[studio.selTrack]) {
            needsRedraw = true;
        }
    }
    prevChainActiveIdx[studio.selTrack] = selTrk.chainActiveIdx;

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

    textX = d.text({ MARGIN + 95, infoY + 4 }, "Engine: ", 8, { .color = { 185, 185, 185 }, .font = &PoppinsLight_8 });
    d.text({ textX, infoY + 4 }, engineRegistry[clip.engineId].name, 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    if (clip.saved && clip.sequence.size() > 0) {
        const int previewCols = 12;
        const int previewRows = 4;
        int padding = 4;
        int previewX = infoRect.position.x + 195;
        int previewY = infoRect.position.y;
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
    }

    if (studio.masterScatter.anyActive()) {
        int latestMode = studio.masterScatter.latestActiveMode;
        if (latestMode >= 0 && latestMode < 4) {
            int usableWidth = winW - (MARGIN * 2);
            int colW = usableWidth / 4;
            for (int i = 0; i < 4; i++) {
                std::string name = studio.masterScatter.getParamName(latestMode, i);
                float val = studio.masterScatter.params[latestMode][i];
                char buf[32];
                std::snprintf(buf, sizeof(buf), "%.2f", val);
                int x = MARGIN + i * colW + 4;
                int y = infoY + 22;
                int nextX = d.text({ x, y }, name + ": ", 8, { .color = { 185, 185, 185 }, .font = &PoppinsLight_8 });
                d.text({ nextX, y }, buf, 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
            }
        }
    } else {
        struct VisualChainItem {
            int originalIndexStart;
            int originalIndexEnd;
            int clipIdx;
            int count;
        };

        std::vector<VisualChainItem> visualItems;
        if (!trk.chain.empty()) {
            int curClip = trk.chain[0];
            int count = 1;
            int startIdx = 0;
            for (size_t i = 1; i < trk.chain.size(); i++) {
                if (trk.chain[i] == curClip) {
                    count++;
                } else {
                    visualItems.push_back({ startIdx, (int)i - 1, curClip, count });
                    curClip = trk.chain[i];
                    count = 1;
                    startIdx = (int)i;
                }
            }
            visualItems.push_back({ startIdx, (int)trk.chain.size() - 1, curClip, count });
        }

        if (!visualItems.empty()) {
            int chainY = infoY + 20;
            int chainH = 12;
            int startX = MARGIN + 4;
            int maxWidth = infoW - 8;

            auto getItemWidth = [](const VisualChainItem& item) {
                if (item.clipIdx == -1) {
                    return item.count > 1 ? 20 : 10;
                } else {
                    return item.count > 1 ? 30 : 16;
                }
            };

            int activeIndex = -1;
            if (trk.chainPlaying && trk.chainActiveIdx >= 0 && trk.chainActiveIdx < (int)trk.chain.size()) {
                for (size_t i = 0; i < visualItems.size(); i++) {
                    if (trk.chainActiveIdx >= visualItems[i].originalIndexStart && trk.chainActiveIdx <= visualItems[i].originalIndexEnd) {
                        activeIndex = (int)i;
                        break;
                    }
                }
            }

            int left = 0;
            int right = (int)visualItems.size() - 1;

            if (trk.chainPlaying && activeIndex != -1) {
                int totalW = getItemWidth(visualItems[activeIndex]);
                left = activeIndex;
                right = activeIndex;
                while (true) {
                    bool expanded = false;
                    if (right + 1 < (int)visualItems.size()) {
                        int w = getItemWidth(visualItems[right + 1]);
                        if (totalW + w + 2 <= maxWidth) {
                            totalW += w + 2;
                            right++;
                            expanded = true;
                        }
                    }
                    if (left - 1 >= 0) {
                        int w = getItemWidth(visualItems[left - 1]);
                        if (totalW + w + 2 <= maxWidth) {
                            totalW += w + 2;
                            left--;
                            expanded = true;
                        }
                    }
                    if (!expanded) break;
                }
            } else {
                int totalW = 0;
                left = (int)visualItems.size() - 1;
                right = (int)visualItems.size() - 1;
                while (left >= 0) {
                    int w = getItemWidth(visualItems[left]);
                    if (totalW + w + 2 <= maxWidth) {
                        totalW += w + 2;
                        left--;
                    } else {
                        break;
                    }
                }
                left++;
            }

            int currentX = startX;
            for (int i = left; i <= right; i++) {
                const auto& item = visualItems[i];
                int itemW = getItemWidth(item);

                Color bg;
                Color textCol = { 255, 255, 255 };
                bool isActive = (i == activeIndex);

                if (item.clipIdx == -1) {
                    bg = { 100, 100, 100 }; // Grey for rest
                } else {
                    bg = trk.themeColor;
                }

                // Draw rectangle
                d.filledRect({ currentX, chainY }, { itemW, chainH }, { .color = bg });

                // If active, highlight
                if (isActive) {
                    d.rect({ currentX, chainY }, { itemW, chainH }, { .color = { 255, 255, 255 } });
                }

                // Display text
                std::string label = "";
                if (item.clipIdx == -1) {
                    if (item.count > 1) {
                        label = "x" + std::to_string(item.count);
                    } else {
                        label = "";
                    }
                } else {
                    if (item.count > 1) {
                        label = std::to_string(item.clipIdx + 1) + " x" + std::to_string(item.count);
                    } else {
                        label = std::to_string(item.clipIdx + 1);
                    }
                }

                if (!label.empty()) {
                    d.textCentered({ currentX + itemW / 2, chainY + 2 }, label, 8, { .color = textCol, .font = &PoppinsLight_8 });
                }

                currentX += itemW + 2;
            }
        }
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

    if (studio.currentCombinationKey == KeyShift) {
        if (key == KEY_1) {
            // Chain placeholder
        } else if (key == KEY_2) {
            // Play / Stop icon
            if (trk.chainPlaying) {
                trk.chainPlaying = false;
                trk.chainMuted = false;
                needFullRedraw = true;
            } else if (!trk.chain.empty()) {
                trk.chainPlaying = true;
                trk.chainActiveIdx = 0;
                trk.chainMuted = false;
                int firstItem = trk.chain[0];
                if (firstItem == -1) {
                    trk.chainMuted = true;
                } else {
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    loadClip(trk, firstItem);
                }
                needFullRedraw = true;
            }
        } else if (key == KEY_3) {
            // Add +
            trk.chain.push_back(selectedClipIdx);
            needsRedraw = true;
        } else if (key == KEY_4) {
            // Rest
            trk.chain.push_back(-1);
            needsRedraw = true;
        } else if (key == KEY_5) {
            // Pop -
            if (!trk.chain.empty()) {
                trk.chain.pop_back();
                if (trk.chain.empty()) {
                    trk.chainPlaying = false;
                    trk.chainMuted = false;
                }
                needsRedraw = true;
            }
        } else if (key == KEY_6) {
            // Clear
            trk.chain.clear();
            trk.chainPlaying = false;
            trk.chainMuted = false;
            needsRedraw = true;
        } else if (key == KEY_7) {
            // Loop
            trk.chainLoopMode = (trk.chainLoopMode == 0) ? 1 : 0;
            needsRedraw = true;
            needFullRedraw = true;
        } else if (key == KEY_8) { // Delete
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
            copiedClip = trk.clips[selectedClipIdx];
            hasCopiedClip = true;
        } else if (key == KEY_F5) { // Paste
            if (hasCopiedClip) {
                trk.clips[selectedClipIdx] = copiedClip;
                if (trk.activeClipIdx == selectedClipIdx) {
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    trk.setEngine(copiedClip.engineId);
                    loadClip(trk, selectedClipIdx);
                }
                needsRedraw = true;
            }
        }
    } else if (key == KEY_1) { // Left
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
        trk.chainPlaying = false;
        trk.chainMuted = false;
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
        needsRedraw = true;
    } else if (key == KEY_6) {
        studio.masterScatter.toggleMode(1);
        needsRedraw = true;
    } else if (key == KEY_7) {
        studio.masterScatter.toggleMode(2);
        needsRedraw = true;
    } else if (key == KEY_8) {
        studio.masterScatter.toggleMode(3);
        needsRedraw = true;
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
        needsRedraw = true;
    } else if (key == KEY_6) {
        studio.masterScatter.toggleMode(1);
        needsRedraw = true;
    } else if (key == KEY_7) {
        studio.masterScatter.toggleMode(2);
        needsRedraw = true;
    } else if (key == KEY_8) {
        studio.masterScatter.toggleMode(3);
        needsRedraw = true;
    }
}

void onEncoder(int encoderId, int8_t direction)
{
    if (studio.currentView != ViewClips) return;

    if (studio.masterScatter.anyActive()) {
        int latestMode = studio.masterScatter.latestActiveMode;
        if (latestMode >= 0 && latestMode < 4) {
            studio.masterScatter.tweakParam(latestMode, encoderId - 1, direction, false);
            needsRedraw = true;
        }
    }
}

bool mouseWheelScrolled(Point position, int delta, const int winW, uint32_t now, bool shifted)
{
    if (studio.currentView != ViewClips) return false;

    if (studio.masterScatter.anyActive()) {
        int latestMode = studio.masterScatter.latestActiveMode;
        if (latestMode >= 0 && latestMode < 4) {
            const int paramsPerRow = 4;
            int usableWidth = winW - (MARGIN * 2);
            int adjustedColW = usableWidth / paramsPerRow;
            int col = (position.x - MARGIN) / adjustedColW;
            if (col >= 0 && col < paramsPerRow) {
                onEncoder(col + 1, delta);
                return true;
            }
        }
    }
    return false;
}
}