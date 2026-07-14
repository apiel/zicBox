#pragma once

#include "draw/utils/inRect.h"
#include "helpers/enc.h"
#include "zicXYv2/draw.h"
#include "zicXYv2/project.h"
#include "zicXYv2/studio.h"
#include <algorithm>
#include <chrono>
#include <cstdint>

namespace UiChain {

bool needsRedraw = true;
int chainSelectedSlot = 0; // selected slot in the active track's chain

bool draw(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY)
{
    if (!needsRedraw && !needFullRedraw) {
        return false;
    }
    needsRedraw = false;

    int top = currentY + 2;

    // Draw background
    d.filledRect({ MARGIN, top }, { winW - (MARGIN * 2), winH - top - MARGIN }, { .color = d.styles.colors.background });

    // Remaining height: winH - top - MARGIN
    int overviewH = 100;
    int rowH = overviewH / MAX_TRACKS; // ~12px per row
    
    d.text({ MARGIN + 4, top + 2 }, "CHANNELS OVERVIEW", 8, { .color = { 150, 150, 150 }, .font = &PoppinsLight_8 });
    
    int startY = top + 14;
    for (int t = 0; t < MAX_TRACKS; t++) {
        if (studio.tracks[t] == nullptr) break;
        Track& trk = *studio.tracks[t];
        int y = startY + t * rowH;

        // Draw track name
        d.text({ MARGIN + 4, y }, "T" + std::to_string(t + 1), 8, { .color = trk.themeColor, .font = &PoppinsLight_8 });

        // Draw compressed chain items
        struct VisualItem {
            int clipIdx;
            int count;
        };
        std::vector<VisualItem> vItems;
        if (!trk.chain.empty()) {
            int curClip = trk.chain[0];
            int count = 1;
            for (size_t i = 1; i < trk.chain.size(); i++) {
                if (trk.chain[i] == curClip) {
                    count++;
                } else {
                    vItems.push_back({ curClip, count });
                    curClip = trk.chain[i];
                    count = 1;
                }
            }
            vItems.push_back({ curClip, count });
        }

        int x = MARGIN + 30;
        int activeIdx = trk.chainPlaying ? trk.chainActiveIdx : -1;
        int curOrigIdx = 0; // to keep track of the original index in the chain
        for (const auto& vi : vItems) {
            bool containsActive = false;
            if (activeIdx >= curOrigIdx && activeIdx < curOrigIdx + vi.count) {
                containsActive = true;
            }
            curOrigIdx += vi.count;

            int itemW = (vi.clipIdx == -1) ? (vi.count > 1 ? 20 : 10) : (vi.count > 1 ? 30 : 16);
            Color bg = (vi.clipIdx == -1) ? Color{ 80, 80, 80 } : trk.themeColor;
            
            d.filledRect({ x, y - 1 }, { itemW, rowH - 2 }, { .color = bg });
            if (containsActive) {
                // Highlight with white border
                d.rect({ x, y - 1 }, { itemW, rowH - 2 }, { .color = { 255, 255, 255 } });
            }

            std::string textVal = "";
            if (vi.clipIdx == -1) {
                textVal = vi.count > 1 ? "x" + std::to_string(vi.count) : "";
            } else {
                textVal = vi.count > 1 ? std::to_string(vi.clipIdx + 1) + "x" + std::to_string(vi.count) : std::to_string(vi.clipIdx + 1);
            }
            if (!textVal.empty()) {
                d.textCentered({ x + itemW / 2, y - 1 }, textVal, 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
            }

            x += itemW + 2;
            if (x >= winW - MARGIN - 10) break; // prevent drawing offscreen
        }
        
        // Highlight active track
        if (studio.selTrack == t) {
            d.rect({ MARGIN, y - 2 }, { winW - MARGIN * 2, rowH }, { .color = { 100, 100, 100 } });
        }
    }

    // Bottom part: Detailed view of the selected track's chain
    Track& trk = *studio.tracks[studio.selTrack];
    int detailY = startY + MAX_TRACKS * rowH + 6;
    int detailH = winH - detailY - MARGIN - 2;

    d.filledRect({ MARGIN, detailY }, { winW - MARGIN * 2, detailH }, { .color = d.styles.colors.quaternary });

    // Draw header for selected track
    std::string trkLabel = "Track " + std::to_string(studio.selTrack + 1) + " Chain Details";
    d.text({ MARGIN + 6, detailY + 4 }, trkLabel, 8, { .color = trk.themeColor, .font = &PoppinsLight_8 });

    std::string modeLabel = trk.chainLoopMode == 1 ? "Mode: HOLD" : "Mode: LOOP";
    d.text({ winW - MARGIN - 80, detailY + 4 }, modeLabel, 8, { .color = { 200, 200, 200 }, .font = &PoppinsLight_8 });

    if (trk.chain.empty()) {
        d.text({ MARGIN + 20, detailY + 22 }, "Chain is empty.", 8, { .color = { 150, 150, 150 }, .font = &PoppinsLight_8 });
        d.text({ MARGIN + 20, detailY + 34 }, "Hold Shift and press 'Add +' in Clip grid view to add clips.", 8, { .color = { 120, 120, 120 }, .font = &PoppinsLight_8 });
    } else {
        int boxW = 22;
        int boxH = 20;
        int boxY = detailY + 18;
        int startX = MARGIN + 6;
        int maxSlotsVisible = (winW - MARGIN * 2 - 12) / (boxW + 2);

        if (chainSelectedSlot >= (int)trk.chain.size()) {
            chainSelectedSlot = (int)trk.chain.size() - 1;
        }
        if (chainSelectedSlot < 0) chainSelectedSlot = 0;

        int scrollOffset = 0;
        if (chainSelectedSlot >= maxSlotsVisible) {
            scrollOffset = chainSelectedSlot - maxSlotsVisible + 1;
        }

        for (int i = 0; i < maxSlotsVisible; i++) {
            int slotIdx = i + scrollOffset;
            if (slotIdx >= (int)trk.chain.size()) break;

            int x = startX + i * (boxW + 2);
            int itemVal = trk.chain[slotIdx];

            Color bg = (itemVal == -1) ? Color{ 80, 80, 80 } : trk.themeColor;
            d.filledRect({ x, boxY }, { boxW, boxH }, { .color = bg });

            // Highlight cursor slot
            if (slotIdx == chainSelectedSlot) {
                d.rect({ x, boxY }, { boxW, boxH }, { .color = { 255, 255, 255 } });
                d.rect({ x + 1, boxY + 1 }, { boxW - 2, boxH - 2 }, { .color = { 255, 255, 255 } });
            }

            // Highlight playing slot
            if (trk.chainPlaying && trk.chainActiveIdx == slotIdx) {
                d.rect({ x - 1, boxY - 1 }, { boxW + 2, boxH + 2 }, { .color = { 0, 255, 0 } });
            }

            std::string valStr = (itemVal == -1) ? "R" : std::to_string(itemVal + 1);
            d.textCentered({ x + boxW / 2, boxY + 6 }, valStr, 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
        }

        // Display current cursor slot details
        std::string slotDetail = "Slot " + std::to_string(chainSelectedSlot + 1) + " of " + std::to_string(trk.chain.size());
        if (trk.chain[chainSelectedSlot] == -1) {
            slotDetail += " : REST (Mute)";
        } else {
            slotDetail += " : Clip " + std::to_string(trk.chain[chainSelectedSlot] + 1);
        }
        d.text({ MARGIN + 6, detailY + 42 }, slotDetail, 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
    }

    return true;
}

void onEncoder(int encoderId, int8_t direction)
{
    if (studio.currentView != ViewChain) return;
    Track& trk = *studio.tracks[studio.selTrack];

    if (encoderId == 1) {
        int newTrack = studio.selTrack + direction;
        if (newTrack >= 0 && newTrack < MAX_TRACKS) {
            studio.selTrack = newTrack;
            chainSelectedSlot = 0;
            needsRedraw = true;
        }
    } else if (encoderId == 2) {
        if (!trk.chain.empty()) {
            int newSlot = chainSelectedSlot + direction;
            if (newSlot >= 0 && newSlot < (int)trk.chain.size()) {
                chainSelectedSlot = newSlot;
                needsRedraw = true;
            }
        }
    } else if (encoderId == 3) {
        if (!trk.chain.empty()) {
            int newVal = trk.chain[chainSelectedSlot] + direction;
            if (newVal >= -1 && newVal < MAX_CLIP_COUNT) {
                trk.chain[chainSelectedSlot] = newVal;
                needsRedraw = true;
            }
        }
    } else if (encoderId == 4) {
        trk.chainLoopMode = (trk.chainLoopMode == 0) ? 1 : 0;
        needsRedraw = true;
    }
}

void keyPressed(int key, bool& needFullRedraw)
{
    if (studio.currentCombinationKey == KeyView) return;
    if (studio.currentView != ViewChain) return;

    Track& trk = *studio.tracks[studio.selTrack];

    if (studio.currentCombinationKey == KeyShift) {
        if (key >= KEY_1 && key <= KEY_8) {
            studio.selTrack = key - KEY_1;
            chainSelectedSlot = 0;
            needsRedraw = true;
        }
        return;
    }

    if (key == KEY_1) { // Left
        if (chainSelectedSlot > 0) {
            chainSelectedSlot--;
            needsRedraw = true;
        }
    } else if (key == KEY_2) { // Right
        if (chainSelectedSlot < (int)trk.chain.size() - 1) {
            chainSelectedSlot++;
            needsRedraw = true;
        }
    } else if (key == KEY_3) { // Insert
        if (trk.chain.empty()) {
            trk.chain.push_back(-1);
        } else {
            trk.chain.insert(trk.chain.begin() + chainSelectedSlot, trk.chain[chainSelectedSlot]);
        }
        needsRedraw = true;
    } else if (key == KEY_4) { // Delete
        if (!trk.chain.empty()) {
            trk.chain.erase(trk.chain.begin() + chainSelectedSlot);
            if (chainSelectedSlot >= (int)trk.chain.size() && !trk.chain.empty()) {
                chainSelectedSlot = (int)trk.chain.size() - 1;
            }
            if (trk.chain.empty()) {
                trk.chainPlaying = false;
                trk.chainMuted = false;
            }
            needsRedraw = true;
        }
    } else if (key == KEY_5) { // Move Left
        if (chainSelectedSlot > 0 && !trk.chain.empty()) {
            std::swap(trk.chain[chainSelectedSlot], trk.chain[chainSelectedSlot - 1]);
            chainSelectedSlot--;
            needsRedraw = true;
        }
    } else if (key == KEY_6) { // Move Right
        if (chainSelectedSlot < (int)trk.chain.size() - 1 && !trk.chain.empty()) {
            std::swap(trk.chain[chainSelectedSlot], trk.chain[chainSelectedSlot + 1]);
            chainSelectedSlot++;
            needsRedraw = true;
        }
    } else if (key == KEY_7) { // Toggle Rest
        if (!trk.chain.empty()) {
            if (trk.chain[chainSelectedSlot] == -1) {
                trk.chain[chainSelectedSlot] = 0;
            } else {
                trk.chain[chainSelectedSlot] = -1;
            }
            needsRedraw = true;
        }
    } else if (key == KEY_8) { // Play/Stop
        if (trk.chainPlaying) {
            trk.chainPlaying = false;
            trk.chainMuted = false;
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
        }
        needsRedraw = true;
    } else if (key == KEY_F2) { // Up
        if (studio.selTrack > 0) {
            studio.selTrack--;
            chainSelectedSlot = 0;
            needsRedraw = true;
        }
    } else if (key == KEY_F3) { // Shift
        studio.currentCombinationKey = KeyShift;
        needFullRedraw = true;
    }
}

void keyReleased(int key, bool& needFullRedraw)
{
    if (studio.currentView != ViewChain) return;

    if (key == KEY_F3) {
        studio.currentCombinationKey = KeyNone;
        needFullRedraw = true;
    }
}

bool mouseButtonPressed(Point position, const int winW, bool& needFullRedraw)
{
    if (studio.currentView != ViewChain) return false;
    return true;
}

bool mouseWheelScrolled(Point position, int delta, const int winW, uint32_t now, bool shifted)
{
    if (studio.currentView != ViewChain) return false;
    onEncoder(2, delta > 0 ? 1 : -1);
    return true;
}

}
