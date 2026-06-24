#pragma once

#include "draw/utils/inRect.h"
#include "zicXYv2/draw.h"
#include "zicXYv2/drawPad.h"
#include "zicXYv2/studio.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace MasterFx {

bool needsRedraw = true;

Rect compRects[4]; // Thresh, Ratio, Attack, Release
Rect compMeterRect;

int paramsTopY = 0; // Track where the grid row starts for context matching

uint8_t encodersSelection = 0;

size_t paramCount = 13;
bool drawStatic(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY)
{
    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;
    Color color = { 0, 180, 255 };

    currentY += 5;
    paramsTopY = currentY; // Store this coordinates for mouse position checking

    const int meterHeight = 8;
    compMeterRect = { { MARGIN, winH - MARGIN - meterHeight }, { winW - (MARGIN * 2), meterHeight } };

    Param params[paramCount] = {
        { .key = "trk1vol", .label = "Track 1", .unit = "%", .value = studio.tracks[0]->volume * 100.0f, .min = 0.0f, .max = 100.0f },
        { .key = "trk2vol", .label = "Track 2", .unit = "%", .value = studio.tracks[1]->volume * 100.0f, .min = 0.0f, .max = 100.0f },
        { .key = "trk3vol", .label = "Track 3", .unit = "%", .value = studio.tracks[2]->volume * 100.0f, .min = 0.0f, .max = 100.0f },
        { .key = "trk4vol", .label = "Track 4", .unit = "%", .value = studio.tracks[3]->volume * 100.0f, .min = 0.0f, .max = 100.0f },
        { .key = "trk5vol", .label = "Track 5", .unit = "%", .value = studio.tracks[4]->volume * 100.0f, .min = 0.0f, .max = 100.0f },
        { .key = "trk6vol", .label = "Track 6", .unit = "%", .value = studio.tracks[5]->volume * 100.0f, .min = 0.0f, .max = 100.0f },
        { .key = "trk7vol", .label = "Track 7", .unit = "%", .value = studio.tracks[6]->volume * 100.0f, .min = 0.0f, .max = 100.0f },
        { .key = "trk8vol", .label = "Track 8", .unit = "%", .value = studio.tracks[7]->volume * 100.0f, .min = 0.0f, .max = 100.0f },

        { .key = "Threshold", .label = "Compressor", .unit = "dB", .value = studio.compressor.threshold, .min = -60.0f, .max = 0.0f },
        { .key = "Ratio", .label = "Comp. Ratio", .unit = ":1", .value = studio.compressor.ratio, .min = 1.0f, .max = 20.0f },
        { .key = "Attack", .label = "Comp. Attack", .unit = "ms", .value = studio.compressor.attack * 1000.f, .min = 1.0f, .max = 100.0f },
        { .key = "Release", .label = "Comp. Release", .unit = "ms", .value = studio.compressor.release * 1000.f, .min = 10.0f, .max = 500.0f },

        { .key = "masterVolume", .label = "Master volume", .unit = "%", .value = studio.volume * 100.0f, .min = 0.0f, .max = 100.0f },
    };
    for (auto& param : params) {
        param.finalize();
    }
    const int paramsPerRow = 4;
    const int colW = (winW - MARGIN * 2) / paramsPerRow;
    UiDraw::params(d, params, paramCount, winW, winH, colW, currentY, paramsPerRow, currentY, color, encodersSelection, 3);
    currentY += UiDraw::ROW_H + 5;

    return true;
}

int lastMeterWidth = -1;
bool drawCompressorMeter(Draw& d, bool needFullRedraw)
{
    float grDb = studio.compressor.getGainReductionDb();
    float grPct = std::clamp(-grDb / 20.0f, 0.0f, 1.0f);
    int fillW = (int)(compMeterRect.size.w * grPct);

    if (fillW != lastMeterWidth || needFullRedraw) {
        lastMeterWidth = fillW;

        d.filledRect(compMeterRect.position, compMeterRect.size, { .color = { 30, 30, 35 } });
        d.filledRect(compMeterRect.position, { fillW, compMeterRect.size.h }, { .color = { 255, 100, 0 } });
        return true;
    }
    return false;
}

bool draw(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY)
{
    bool rendered = false;
    rendered |= drawStatic(d, winW, winH, needFullRedraw, currentY);
    rendered |= drawCompressorMeter(d, needFullRedraw);
    return rendered;
}

bool mouseWheelScrolled(Point position, int delta, const int winW, uint32_t now, bool shifted)
{
    if (studio.currentView != ViewMaster) return false;

    const int paramsPerRow = 4;
    const int maxVisibleRows = 3;
    const int SB_WIDTH = 3;
    const int SB_GAP = 1;

    int usableWidth = winW - (MARGIN * 2) - (SB_WIDTH + SB_GAP);
    int adjustedColW = usableWidth / paramsPerRow;

    int visualRow = (position.y - paramsTopY) / UiDraw::ROW_H;
    int col = (position.x - MARGIN) / adjustedColW;

    size_t totalParamRows = ((int)paramCount + paramsPerRow - 1) / paramsPerRow;

    int startRow = 0;
    int activeRow = encodersSelection;

    if (activeRow < startRow) {
        startRow = activeRow;
    } else if (activeRow >= startRow + maxVisibleRows) {
        startRow = activeRow - maxVisibleRows + 1;
    }

    if (startRow > (int)totalParamRows - maxVisibleRows) {
        startRow = std::max(0, (int)totalParamRows - maxVisibleRows);
    }

    if (visualRow >= 0 && visualRow < maxVisibleRows && col >= 0 && col < paramsPerRow) {
        int absoluteRow = startRow + visualRow;
        size_t paramIndex = (absoluteRow * paramsPerRow) + col;

        if (paramIndex < paramCount) {
            float direction = (delta > 0) ? 1.0f : -1.0f;

            if (paramIndex <= 7) {
                float step = 0.5f;
                studio.tracks[paramIndex]->volume = std::clamp(studio.tracks[paramIndex]->volume + (direction * step), 0.0f, 1.0f);
                encodersSelection = absoluteRow;
            } else if (paramIndex == 8) {
                float step = shifted ? 5.0f : 1.0f;
                studio.compressor.threshold = std::clamp(studio.compressor.threshold + (direction * step), -60.0f, 0.0f);
                encodersSelection = absoluteRow;
            } else if (paramIndex == 9) {
                float step = shifted ? 2.0f : 0.5f;
                studio.compressor.ratio = std::clamp(studio.compressor.ratio + (direction * step), 1.0f, 20.0f);
                encodersSelection = absoluteRow;
            } else if (paramIndex == 10) {
                float step = (shifted ? 10.0f : 1.0f) / 1000.0f;
                studio.compressor.attack = std::clamp(studio.compressor.attack + (direction * step), 0.001f, 0.1f);
                encodersSelection = absoluteRow;
            } else if (paramIndex == 11) {
                float step = (shifted ? 50.0f : 10.0f) / 1000.0f;
                studio.compressor.release = std::clamp(studio.compressor.release + (direction * step), 0.01f, 0.5f);
                encodersSelection = absoluteRow;
            } else if (paramIndex == 12) {
                float step = (shifted ? 5.0f : 1.0f) / 100.0f;
                studio.volume = std::clamp(studio.volume + (direction * step), 0.0f, 1.0f);
                encodersSelection = absoluteRow;
            }

            needsRedraw = true;
            return true;
        }
    }

    return false;
}

} // namespace MasterFx