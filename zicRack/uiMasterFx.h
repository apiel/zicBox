#pragma once

#include "draw/utils/inRect.h"
#include "zicRack/drawPad.h"
#include "zicRack/studio.h"
#include "zicRack/draw.h"

namespace MasterFx {

bool needsRedraw = true;

bool filterDragging = false;

Rect filterPadRect;

Rect compRects[4]; // Thresh, Ratio, Attack, Release
Rect compMeterRect;

bool drawStatic(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY)
{
    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;
    Color color = { 0, 180, 255 };

    currentY += 5;

    size_t paramCount = 13;
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
    for (auto &param : params) { param.finalize(); }
    const int paramsPerRow = 8;
    const int colW = (winW - MARGIN * 2) / paramsPerRow;
    UiDraw::params(d, params, paramCount, winW, winH, colW, currentY, paramsPerRow, currentY, color, 0);
    currentY += UiDraw::ROW_H + 5;

    int padW = colW * 4, padH = winH - currentY - 5;
    filterPadRect = { { MARGIN, currentY }, { padW, padH } };
    float fx = (studio.filter.getCutoff() + 1.0f) * 0.5f;
    float fy = 1.0f - studio.filter.getResonance();
    drawPad(d, filterPadRect, "MASTER FILTER", color, fx, fy);

    // Compressor UI (Right side of pad)
    int compX = filterPadRect.position.x + padW + MARGIN;
    int compW = 150;
    d.text({ compX, currentY }, "COMPRESSOR", 8, { .color = color, .font = &PoppinsLight_8 });

    auto drawParam = [&](int idx, std::string label, float val, float min, float max, std::string unit) {
        int py = currentY + 15 + idx * 25;
        Point position = { compX, py };
        Size size = { compW - 30, 20 };
        compRects[idx] = { position, size };
        d.filledRect(position, size, { .color = { 30, 30, 35 } });
        float pct = (val - min) / (max - min);
        d.filledRect({ position.x, position.y + 16 }, { (int)(size.w * pct), 2 }, { .color = color });
        d.text({ position.x + 2, position.y + 2 }, label, 8, { .color = { 200, 200, 200 }, .font = &PoppinsLight_8 });
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << val << unit;
        d.textRight({ position.x + size.w - 2, position.y + 2 }, ss.str(), 8, { .color = { 150, 150, 150 }, .font = &PoppinsLight_8 });
    };

    drawParam(0, "Threshold", studio.compressor.threshold, -60.0f, 0.0f, "dB");
    drawParam(1, "Ratio", studio.compressor.ratio, 1.0f, 20.0f, ":1");
    drawParam(2, "Attack", studio.compressor.attack * 1000.f, 1.0f, 100.0f, "ms");
    drawParam(3, "Release", studio.compressor.release * 1000.f, 10.0f, 500.0f, "ms");

    int meterX = compX + compW - 20;
    int meterH = padH - 20;
    compMeterRect = { { meterX, currentY + 15 }, { 10, meterH } };

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

void mouseMoved(Point position)
{
    if (studio.currentView != ViewMaster) return;

    if (filterDragging) {
        onFilterPad(position);
    }
}

void mouseButtonPressed(Point position)
{
    if (studio.currentView != ViewMaster) return;

    if (inRect(filterPadRect, position)) {
        filterDragging = true;
        onFilterPad(position);
    }
}

void mouseButtonReleased()
{
    filterDragging = false;
}

bool mouseWheelScrolled(Point position, int delta, bool shifted)
{
    if (studio.currentView != ViewMaster) return false;

    for (int i = 0; i < 4; i++) {
        if (inRect(compRects[i], position)) {
            float step = shifted ? 5.0f : 1.0f;
            if (i == 0) studio.compressor.threshold = std::clamp(studio.compressor.threshold + (delta > 0 ? step : -step), -60.0f, 0.0f);
            else if (i == 1) studio.compressor.ratio = std::clamp(studio.compressor.ratio + (delta > 0 ? 0.5f : -0.5f), 1.0f, 20.0f);
            else if (i == 2) studio.compressor.attack = std::clamp(studio.compressor.attack + (delta > 0 ? 0.001f : -0.001f), 0.0001f, 0.1f);
            else if (i == 3) studio.compressor.release = std::clamp(studio.compressor.release + (delta > 0 ? 0.01f : -0.01f), 0.01f, 1.0f);
            needsRedraw = true;
            return true;
        }
    }
    return false;
}

}