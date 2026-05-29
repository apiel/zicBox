#pragma once

#include "draw/utils/inRect.h"
#include "helpers/format.h" // For fToString if you want to reuse it, or use std::stringstream
#include "zicRack/studio.h"
#include <algorithm>
#include <string>

namespace UiTrackShift {

bool needsRedraw = true;
static constexpr int ROW_H = 50; // Larger rows for clear encoder blocks
Rect encoderRects[8];

bool draw(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY)
{
    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;

    Track& trk = *studio.tracks[studio.selTrack];

    // Fill background with a dark tint overlaying the track space
    int totalW = winW - (MARGIN * 2);
    int totalH = winH - currentY - 50;
    d.filledRect({ MARGIN, currentY }, { totalW, totalH }, { .color = { 20, 20, 25, 235 } }); // High alpha overlay

    currentY += 15;

    // 2 rows of 4 encoders layout
    int cols = 4;
    int rows = 2;
    int cellW = totalW / cols;
    int cellH = ROW_H;

    for (int i = 0; i < 8; i++) {
        int r = i / cols;
        int c = i % cols;

        int ex = MARGIN + c * cellW;
        int ey = currentY + r * (cellH + 10); // 10px spacing between rows

        encoderRects[i] = { { ex, ey }, { cellW - 8, cellH } };

        // Draw encoder box background
        Color boxBg = { 35, 35, 40 };
        d.filledRect(encoderRects[i].position, encoderRects[i].size, { .color = boxBg });

        // Slot Labels and assignments
        std::string label;
        std::string value = "---";

        if (i == 0) {
            label = "SYNTH ENGINE";
            int currentEngineIdx = trk.currentEngineIdx;
            value = engineRegistry[currentEngineIdx].name;
        } else if (i == 3) { // Top Right Encoder
            label = "TRACK VOLUME";
            // Displays volume as a percentage (e.g., "80%")
            value = std::to_string((int)(trk.volume * 100)) + "%";
        } else {
            label = "RESERVED";
        }

        d.text({ ex + 8, ey + 6 }, label, 8, { .color = { 150, 150, 160 }, .font = &PoppinsLight_8 });
        d.text({ ex + 8, ey + 22 }, value, 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });

        // Optional: Draw a tiny visual feedback bar for volume inside index 3
        if (i == 3) {
            int barW = (int)((cellW - 24) * trk.volume);
            d.filledRect({ ex + 8, ey + ROW_H - 6 }, { barW, 2 }, { .color = trk.themeColor });
        }

        // Structural boundary accent
        // d.rect(encoderRects[i].position, encoderRects[i].size, { .color = trk.themeColor });
    }

    return true;
}

bool mouseWheelScrolled(Point position, int delta)
{
    int sc = delta > 0 ? 1 : -1;

    for (int i = 0; i < 8; i++) {
        if (inRect(encoderRects[i], position)) {
            Track& trk = *studio.tracks[studio.selTrack];

            if (i == 0) { // First encoder: Synth Engine Selector
                std::lock_guard<std::mutex> lock(studio.audioMutex);

                int currentEngineIdx = trk.currentEngineIdx;
                currentEngineIdx += sc;

                // Safe wrapping boundary check
                if (currentEngineIdx >= (int)ENGINE_REGISTRY_COUNT) currentEngineIdx = 0;
                if (currentEngineIdx < 0) currentEngineIdx = (int)ENGINE_REGISTRY_COUNT - 1;

                // Update the track engine
                trk.setEngine(currentEngineIdx);

                needsRedraw = true;
                return true;
            } else if (i == 3) { // Fourth encoder (Top Right): Volume Control
                std::lock_guard<std::mutex> lock(studio.audioMutex);

                // Adjust volume steps (e.g., 2% or 0.02f increments per tick)
                // Use a smaller increment if you want fine control
                float volumeStep = 0.02f;
                trk.volume += sc * volumeStep;

                // Clamp volume tightly between 0.0f (silence) and 1.0f (unity/max)
                trk.volume = std::clamp(trk.volume, 0.0f, 1.0f);

                needsRedraw = true;
                return true;
            }
        }
    }
    return false;
}

} // namespace UiTrackShift