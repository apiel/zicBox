#pragma once

#include "zic23/studio.h"
#include "zic23/uiClip.h"
#include "zic23/uiEq.h"

int drawTracks(Draw& d, sf::Vector2u size, int currentY)
{
    const int winW = (int)size.x;
    const int paramsPerRow = 8;
    const int colW = (winW - MARGIN * 2) / paramsPerRow;
    auto now = std::chrono::steady_clock::now();

    int trackIdx = 0;
    for (auto& trkPtr : studio.tracks) {
        Track& trk = *trkPtr;
        int startY = currentY;

        // Track Header
        d.filledRect({ MARGIN, currentY + 2 }, { colW / 2, TRACK_H - 4 }, { .color = d.styles.colors.quaternary });
        d.text({ MARGIN + 4, currentY + 4 }, trk.engine->getName(), 8, { .color = trk.themeColor, .font = &PoppinsLight_8 });

        int vuX = MARGIN + colW / 2 + 4;
        trk.vuRect = sf::IntRect(vuX, currentY + 2, WAVE_HISTORY, TRACK_H - 4);

        int specX = vuX + WAVE_HISTORY + 4;
        specRects[trackIdx] = sf::IntRect(specX, currentY + 2, SPEC_W, TRACK_H - 4);

        int clipStartX = specX + SPEC_W + 10;
        drawClipSelectorUI(d, size, currentY, clipStartX, trackIdx, trk);

        trackIdx++;
        currentY += TRACK_H;

        // Param rows
        Param* params = trk.engine->getParams();
        size_t paramCount = trk.engine->getParamCount();
        Color bgColor = darken(d.styles.colors.quaternary, 0.1);

        for (size_t p = 0; p < paramCount; p++) {
            int x = MARGIN + ((int)p % paramsPerRow) * colW;
            int y = currentY + ((int)p / paramsPerRow) * ROW_H;

            // --- MODULE LOGIC: MODULE_OSC_WAVETABLE ---
            // We check if this is the start of a Wavetable pair (Select + Morph)
            // Conditions: Current is WT, Next is WT, and Current isn't at the very end of a row.
            bool isWavetablePair = (p + 1 < paramCount) && (params[p].module == MODULE_OSC_WAVETABLE) && (params[p + 1].module == MODULE_OSC_WAVETABLE) && ((int)p % paramsPerRow != (paramsPerRow - 1));

            if (isWavetablePair) {
                int combinedW = (colW * 2) - 2;
                d.filledRect({ x, y }, { combinedW, ROW_H - 2 }, { .color = bgColor });

                std::vector<Point> points;
                if (params[p].graph != nullptr) {
                    for (int gx = 4; gx < combinedW - 4; gx++) {
                        float phase = (float)(gx - 4) / (float)(combinedW - 8);
                        float sVal = params[p].getGraphPoint(phase);

                        int centerY = y + (ROW_H / 2);
                        int drawY = centerY - (int)(sVal * (ROW_H / 2.5f));
                        points.push_back({ x + gx, drawY });
                    }
                }
                Color c = trk.themeColor;
                c.a = 80;
                d.lines(points, { .color = c });

                // Labels for the combined block
                d.text({ x + 4, y + 2 }, params[p].string ? params[p].string : params[p].label, 12, { .color = d.styles.colors.text, .font = &PoppinsLight_12 });

                std::stringstream ss;
                ss << "Morph: " << (int)params[p + 1].value;
                d.textRight({ x + combinedW - 6, y + 2 }, ss.str(), 8, { .color = { 150, 150, 150 }, .font = &PoppinsLight_8 });

                p++; // Skip the next param index since we handled the Morph here
            }
            // --- DEFAULT RENDERING ---
            else {
                d.filledRect({ x, y }, { colW - 2, ROW_H - 2 }, { .color = bgColor });
                d.text({ x + 4, y + 2 }, params[p].label, 12, { .color = d.styles.colors.text, .font = &PoppinsLight_12 });

                if (winW >= 900 || (trk.activeParamIdx == (int)p && std::chrono::duration_cast<std::chrono::milliseconds>(now - trk.lastEditTime).count() < 1500)) {
                    std::stringstream ss;
                    ss << std::fixed << std::setprecision(params[p].precision) << params[p].value << params[p].unit;
                    d.textRight({ x + colW - 6, y + 2 }, params[p].string ? params[p].string : ss.str(), 8, { .color = { 120, 120, 130 }, .font = &PoppinsLight_8 });
                }

                float range = params[p].max - params[p].min;
                float pct = (params[p].value - params[p].min) / (range <= 0 ? 1.f : range);
                int bX = x + 4, bY = y + ROW_H - 8, bW = colW - 10;

                if (params[p].type & VALUE_CENTERED) {
                    int mid = bX + bW / 2;
                    int fw = (int)((bW / 2) * (params[p].value / params[p].max));
                    if (fw < 0) d.filledRect({ mid + fw, bY }, { std::abs(fw), 3 }, { .color = trk.themeColor });
                    else d.filledRect({ mid, bY }, { fw, 3 }, { .color = trk.themeColor });
                    d.filledRect({ mid, bY - 1 }, { 1, 5 }, { .color = { 100, 100, 100 } });
                } else {
                    d.filledRect({ bX, bY }, { (int)(bW * pct), 3 }, { .color = trk.themeColor });
                }
            }
        }

        int secH = (((int)trk.engine->getParamCount() + 7) / 8) * ROW_H + TRACK_H;
        trk.trackBounds = sf::IntRect(MARGIN, startY, winW - MARGIN * 2, secH);
        currentY += secH - TRACK_H + 2;
    }

    return currentY;
}