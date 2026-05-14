#pragma once

#include "draw/utils/inRect.h"
#include "helpers/enc.h"
#include "zicRack/studio.h"

namespace UiTrack {

bool needsRedraw = true;
int paramsTopY = 0;

static constexpr int ROW_H = 36; // param panel row height

void drawWaveform(Draw& d, Track& trk, int x, int y, int w, int h)
{
    d.filledRect({ x, y }, { w, h }, { .color = darken(trk.themeColor, 0.9f) });
    
    std::vector<Point> points;
    int centerY = y + (h / 2);
    float amplitude = h * 0.45f;

    for (int i = 0; i < w; i++) {
        float phase = (float)i / (float)w;
        float sample = trk.engine->draw(phase); 
        
        int drawY = centerY - (int)(sample * amplitude);
        points.push_back({ x + i, drawY });
    }

    d.lines(points, { .color = trk.themeColor });
    
    d.rect({ x, y }, { w, h }, { .color = { 255, 255, 255, 20 } });
}

void drawGraph(Draw& d, Track& trk, Param& param, const int colW, int x, int y, Color& bgColor)
{
    std::vector<Point> points;
    int innerW = colW - 10;
    for (int gx = 0; gx < innerW; gx++) {
        float phase = (float)gx / (float)innerW;
        float sVal = param.getGraphPoint(phase);
        int centerY = y + (ROW_H / 2) + 4;
        int drawY = centerY - (int)(sVal * (ROW_H / 5.0f));
        points.push_back({ x + 4 + gx, drawY });
    }
    Color c = trk.themeColor;
    d.lines(points, { .color = c });
    c.a = 50;
    d.filledPolygon(points, { .color = c });
}

void drawParam(Draw& d, Track& trk, Param* params, size_t& p, const int colW, const int winW, int x, int y, Color& bgColor, const std::chrono::steady_clock::time_point& now)
{
    d.filledRect({ x, y }, { colW - 2, ROW_H - 2 }, { .color = bgColor });
    d.text({ x + 4, y + 2 }, params[p].label, 12, { .color = d.styles.colors.text, .font = &PoppinsLight_12 });

    std::stringstream ss;
    if (params[p].string) {
        ss << params[p].string;
    } else {
        ss << std::fixed << std::setprecision(params[p].precision) << params[p].value << params[p].unit;
    }

    d.text({ x + 4, y + 16 }, ss.str(), 8, { .color = { 170, 170, 180 }, .font = &PoppinsLight_8, .maxWidth = colW - 8 });

    float range = params[p].max - params[p].min;
    float pct = (params[p].value - params[p].min) / (range <= 0 ? 1.f : range);
    int bX = x + 4, bY = y + ROW_H - 8, bW = colW - 10;

    if (params[p].graph != nullptr) {
        drawGraph(d, trk, params[p], colW, x, y, bgColor);
    } else if (params[p].type & VALUE_CENTERED) {
        int mid = bX + bW / 2;
        int fw = (int)((bW / 2) * (params[p].value / (params[p].max == 0 ? 1.0f : params[p].max)));

        if (fw < 0) d.filledRect({ mid + fw, bY }, { std::abs(fw), 3 }, { .color = trk.themeColor });
        else d.filledRect({ mid, bY }, { fw, 3 }, { .color = trk.themeColor });

        d.filledRect({ mid, bY - 1 }, { 1, 5 }, { .color = { 100, 100, 100 } });
    } else {
        d.filledRect({ bX, bY }, { (int)(bW * pct), 3 }, { .color = trk.themeColor });
    }
}

bool draw(Draw& d, const int winW, bool needFullRedraw, int currentY)
{
    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;

    if (studio.tracks[studio.selTrack] == nullptr) return false;
    Track& trk = *studio.tracks[studio.selTrack];

    const int paramsPerRow = 8;
    const int colW = (winW - MARGIN * 2) / paramsPerRow;
    auto now = std::chrono::steady_clock::now();

    int waveformH = 60;
    int headerW = winW - (MARGIN * 2);
    drawWaveform(d, trk, MARGIN, currentY, headerW, waveformH);
    
    paramsTopY = currentY + waveformH + MARGIN; 

    Param* params = trk.engine->getParams();
    size_t paramCount = trk.engine->getParamCount();

    Color bgColor = darken(d.styles.colors.quaternary, 0.1);
    int visualIdx = 0;
    for (size_t p = 0; p < paramCount; p++, visualIdx++) {
        int x = MARGIN + ((int)visualIdx % paramsPerRow) * colW;
        int y = paramsTopY + ((int)visualIdx / paramsPerRow) * ROW_H;

        bool isADSR = (p + 3 < paramCount) && (params[p].module == MODULE_ENV_ADSR && params[p + 3].module == MODULE_ENV_ADSR); // could add && params[p + 1].module == MODULE_ENV_ADSR && params[p + 2].module == MODULE_ENV_ADSR && but i guess it s enough..

        drawParam(d, trk, params, p, colW, winW, x, y, bgColor, now);
    }

    return true;
}

void mouseButtonPressed(Point position)
{
}

bool mouseWheelScrolled(Point position, int delta, const int winW, uint32_t now, bool shifted)
{
    if (studio.currentView != ViewTrack) return false;

    if (studio.tracks[studio.selTrack] == nullptr) return false;
    Track& trk = *studio.tracks[studio.selTrack];

    const int cW = (winW - MARGIN * 2) / 8;

    int finalPIdx = ((position.y - paramsTopY) / ROW_H) * 8 + (position.x - MARGIN) / cW;

    if (finalPIdx >= 0 && (size_t)finalPIdx < trk.engine->getParamCount()) {
        std::lock_guard<std::mutex> lock(studio.audioMutex);
        Param& p = trk.engine->getParams()[finalPIdx];

        int scaled = encGetScaledDirection(delta, now, trk.lastShiftTicks[finalPIdx]);
        trk.lastShiftTicks[finalPIdx] = now;

        p.inc(scaled * (shifted ? 5.f : 1.f));

        trk.activeParamIdx = finalPIdx;
        trk.lastEditTime = std::chrono::steady_clock::now();
        needsRedraw = true;
        return true;
    }

    return false;
}

}