#pragma once

#include "draw/utils/inRect.h"
#include "zicRack/draw.h"
#include "zicRack/drawPad.h"
#include "zicRack/studio.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace MasterFx {

bool needsRedraw = true;

bool filterDragging = false;
bool scatterDragging = false; // Added tracking flag for scatter pad

Rect filterPadRect;
Rect scatterPadRect; // Added tracking rect for scatter pad

Rect compRects[4]; // Thresh, Ratio, Attack, Release
Rect compMeterRect;

int paramsTopY = 0; // Track where the grid row starts for context matching

size_t paramCount = 13;
bool drawStatic(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY)
{
    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;
    Color color = { 0, 180, 255 };

    currentY += 5;
    paramsTopY = currentY; // Store this coordinates for mouse position checking

    compMeterRect = { { winW - 10, currentY }, { 5, UiDraw::ROW_H * 2 } };

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
    const int paramsPerRow = 8;
    const int colW = (winW - MARGIN * 2) / paramsPerRow;
    UiDraw::params(d, params, paramCount, winW, winH, colW, currentY, paramsPerRow, currentY, color, 0);
    currentY += UiDraw::ROW_H + 5;

    // Left Side Pad: Filter
    int padW = colW * 4, padH = winH - currentY - 5;
    filterPadRect = { { MARGIN, currentY }, { padW, padH } };
    float fx = (studio.filter.getCutoff() + 1.0f) * 0.5f;
    float fy = 1.0f - studio.filter.getResonance();
    drawPad(d, filterPadRect, "MASTER FILTER", color, fx, fy);

    // Right Side Pad: Scatter Effect
    scatterPadRect = { { MARGIN + padW + MARGIN, currentY }, { padW, padH } };
    // float sx = studio.scatter.getAmount();
    // float sy = studio.scatter.getFeedback();
    float sx = 0.5f;
    float sy = 0.5f;
    drawPad(d, scatterPadRect, "SCATTER FX", { 255, 100, 0 }, sx, sy);

    // Old manual Compressor UI elements removed since they are now drawn via UiDraw::params above

    return true;
}

int lastCutoffY = -1;
bool drawCompressorMeter(Draw& d, bool needFullRedraw)
{
    float grDb = studio.compressor.getGainReductionDb();
    float grPct = std::clamp(-grDb / 20.0f, 0.0f, 1.0f);
    int cutoffY = compMeterRect.size.h - (int)(compMeterRect.size.h * grPct);

    if (cutoffY != lastCutoffY || needFullRedraw) {
        lastCutoffY = cutoffY;

        d.filledRect(compMeterRect.position, compMeterRect.size, { .color = { 30, 30, 35 } });
        d.filledRect({ compMeterRect.position.x, compMeterRect.position.y + cutoffY }, { compMeterRect.size.w, compMeterRect.size.h - cutoffY }, { .color = { 255, 100, 0 } });
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

void onFilterPad(Point position)
{
    float x = position.x - filterPadRect.position.x;
    float y = position.y - filterPadRect.position.y;
    studio.filter.setCutoff(std::clamp((x / filterPadRect.size.w) * 2.0f - 1.0f, -1.0f, 1.0f));
    studio.filter.setResonance(std::clamp(1.0f - (y / filterPadRect.size.h), 0.0f, 1.0f));
    needsRedraw = true;
}

void onScatterPad(Point position)
{
    float x = position.x - scatterPadRect.position.x;
    float y = position.y - scatterPadRect.position.y;
    // studio.scatter.setAmount(std::clamp(x / scatterPadRect.size.w, 0.0f, 1.0f));
    // studio.scatter.setFeedback(std::clamp(1.0f - (y / scatterPadRect.size.h), 0.0f, 1.0f));
    needsRedraw = true;
}

void mouseMoved(Point position)
{
    if (studio.currentView != ViewMaster) return;

    if (filterDragging) {
        onFilterPad(position);
    } else if (scatterDragging) {
        onScatterPad(position);
    }
}

void mouseButtonPressed(Point position)
{
    if (studio.currentView != ViewMaster) return;

    if (inRect(filterPadRect, position)) {
        filterDragging = true;
        onFilterPad(position);
    } else if (inRect(scatterPadRect, position)) {
        scatterDragging = true;
        onScatterPad(position);
    }
}

void mouseButtonReleased()
{
    filterDragging = false;
    scatterDragging = false;
}

bool mouseWheelScrolled(Point position, int delta, const int winW, uint32_t now, bool shifted)
{
    if (studio.currentView != ViewMaster) return false;

    const int paramsPerRow = 8;
    const int cW = (winW - MARGIN * 2) / paramsPerRow;

    int row = (position.y - paramsTopY) / UiDraw::ROW_H;
    int col = (position.x - MARGIN) / cW;

    // Ensure user is within the parameter grid block boundaries
    if (row >= 0 && col >= 0 && col < paramsPerRow) {
        // Flat array calculation matching UiDraw linear assignment layout
        // size_t paramIndex = (row * paramsPerRow) + col;

        // Apply the layout mapping step to find the parameter index
        int blockRow = row / 2;
        int subRow = row % 2;
        int blockSide = col / 4;
        int subCol = col % 4;

        size_t paramIndex = (blockRow * 16) + (blockSide * 8) + (subRow * 4) + subCol;

        if (paramIndex < paramCount) {
            float direction = (delta > 0) ? 1.0f : -1.0f;

            // Track Volumes (Indices 0 - 7)
            if (paramIndex >= 0 && paramIndex <= 7) {
                float step = (shifted ? 5.0f : 1.0f) / 100.0f; // 1% or 5% steps
                studio.tracks[paramIndex]->volume = std::clamp(studio.tracks[paramIndex]->volume + (direction * step), 0.0f, 1.0f);
            }
            // Compressor Threshold (Index 8)
            else if (paramIndex == 8) {
                float step = shifted ? 5.0f : 1.0f;
                studio.compressor.threshold = std::clamp(studio.compressor.threshold + (direction * step), -60.0f, 0.0f);
            }
            // Compressor Ratio (Index 9)
            else if (paramIndex == 9) {
                float step = shifted ? 2.0f : 0.5f;
                studio.compressor.ratio = std::clamp(studio.compressor.ratio + (direction * step), 1.0f, 20.0f);
            }
            // Compressor Attack (Index 10)
            else if (paramIndex == 10) {
                float step = (shifted ? 10.0f : 1.0f) / 1000.0f; // convert back to seconds
                studio.compressor.attack = std::clamp(studio.compressor.attack + (direction * step), 0.001f, 0.1f);
            }
            // Compressor Release (Index 11)
            else if (paramIndex == 11) {
                float step = (shifted ? 50.0f : 10.0f) / 1000.0f; // convert back to seconds
                studio.compressor.release = std::clamp(studio.compressor.release + (direction * step), 0.01f, 0.5f);
            }
            // Master Volume (Index 12)
            else if (paramIndex == 12) {
                float step = (shifted ? 5.0f : 1.0f) / 100.0f;
                studio.volume = std::clamp(studio.volume + (direction * step), 0.0f, 1.0f);
            }

            needsRedraw = true;
            return true;
        }
    }

    return false;
}

} // namespace MasterFx