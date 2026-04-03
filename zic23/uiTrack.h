#pragma once

#include "helpers/enc.h"
#include "zic23/studio.h"
#include "zic23/uiClip.h"
#include "zic23/uiEq.h"

void drawWavetable(Draw& d, Track& trk, Param* params, size_t& p, const int colW, int x, int y, Color& bgColor)
{
    int cellW = colW - 2;
    d.filledRect({ x, y }, { cellW, ROW_H - 2 }, { .color = bgColor });

    // d.line({ x + cellW / 2, y + 3 }, { x + cellW / 2, y + ROW_H - 5 }, { .color = { 50, 50, 50 } });

    if (params[p].graph != nullptr) {
        std::vector<Point> points;
        int innerW = cellW - 8;
        for (int gx = 0; gx < innerW; gx++) {
            float phase = (float)gx / (float)innerW;
            float sVal = params[p].getGraphPoint(phase);
            int centerY = y + (ROW_H / 2) + 4;
            int drawY = centerY - (int)(sVal * (ROW_H / 5.0f));
            points.push_back({ x + 4 + gx, drawY });
        }
        d.lines(points, { .color = trk.themeColor });
    }

    // Left: File Name | Right: Morph Value
    d.text({ x + 4, y + 2 }, params[p].string ? params[p].string : params[p].label, 8, { .color = d.styles.colors.text, .font = &PoppinsLight_8 });

    std::stringstream ss;
    ss << "M:" << (int)params[p + 1].value;
    d.textRight({ x + cellW - 4, y + 2 }, ss.str(), 8, { .color = { 180, 180, 180 }, .font = &PoppinsLight_8 });

    trk.scrollParamIndex.push_back({ p, 2 });
    p++; // Skip the next param index since we handled the Morph here
}

void drawADSR(Draw& d, Track& trk, Param* params, size_t& p, const int colW, int x, int y, Color& bgColor)
{
    int combinedW = (colW * 2) - 2; // Spans 2 slots
    d.filledRect({ x, y }, { combinedW, ROW_H - 2 }, { .color = bgColor });

    int colW1_4 = combinedW / 8;
    // d.textCentered({ x + colW1_4, y + 2 }, "A", 16, { .color = { 100, 100, 100 }, .font = &PoppinsLight_16 });
    // d.textCentered({ x + colW1_4 * 3, y + 2 }, "D", 16, { .color = { 100, 100, 100 }, .font = &PoppinsLight_16 });
    // d.textCentered({ x + colW1_4 * 5, y + 2 }, "S", 16, { .color = { 100, 100, 100 }, .font = &PoppinsLight_16 });
    // d.textCentered({ x + colW1_4 * 7, y + 2 }, "R", 16, { .color = { 100, 100, 100 }, .font = &PoppinsLight_16 });

    Color adsrColor = { 100, 100, 100 };
    Color valueColor = { 170, 170, 170 };
    int top = y + 6;
    int tx = d.text({ x + 6, top }, "A", 16, { .color = adsrColor, .font = &PoppinsLight_16 });
    d.text({ tx + 2, top + 8 }, std::to_string((int)params[p].value) + params[p].unit, 8, { .color = valueColor, .font = &PoppinsLight_8 });

    tx = d.text({ x + colW1_4 * 2 + 6, top }, "D", 16, { .color = adsrColor, .font = &PoppinsLight_16 });
    d.text({ tx + 2, top + 8 }, std::to_string((int)params[p + 1].value) + params[p + 1].unit, 8, { .color = valueColor, .font = &PoppinsLight_8 });

    tx = d.text({ x + colW1_4 * 4 + 6, top }, "S", 16, { .color = adsrColor, .font = &PoppinsLight_16 });
    d.text({ tx + 2, top + 8 }, std::to_string((int)params[p + 2].value) + params[p + 2].unit, 8, { .color = valueColor, .font = &PoppinsLight_8 });

    tx = d.text({ x + colW1_4 * 6 + 6, top }, "R", 16, { .color = adsrColor, .font = &PoppinsLight_16 });
    d.text({ tx + 2, top + 8 }, std::to_string((int)params[p + 3].value) + params[p + 3].unit, 8, { .color = valueColor, .font = &PoppinsLight_8 });

    // Get the 4 values (0.0 - 1.0 normalized)
    float a = (params[p].value - params[p].min) / (params[p].max - params[p].min);
    float d_val = (params[p + 1].value - params[p + 1].min) / (params[p + 1].max - params[p + 1].min);
    float s = (params[p + 2].value - params[p + 2].min) / (params[p + 2].max - params[p + 2].min);
    float r = (params[p + 3].value - params[p + 3].min) / (params[p + 3].max - params[p + 3].min);

    // Render Envelope Shape
    int h = ROW_H - 12;
    int baseLine = y + ROW_H - 6;
    int peakY = y + 6;
    int sustainY = baseLine - (int)(s * h);

    std::vector<Point> points;
    int segW = combinedW / 4;
    points.push_back({ x + 4, baseLine }); // Start
    points.push_back({ x + 4 + (int)(a * segW), peakY }); // Attack Peak
    points.push_back({ x + segW + (int)(d_val * segW), sustainY }); // Decay to Sustain
    points.push_back({ x + (segW * 3), sustainY }); // Sustain Hold
    points.push_back({ x + (segW * 3) + (int)(r * segW), baseLine }); // Release End

    d.lines(points, { .color = trk.themeColor });

    d.filledCircle(points[1], 2, { .color = trk.themeColor });
    d.filledCircle(points[2], 2, { .color = trk.themeColor });
    d.filledCircle(points[3], 2, { .color = trk.themeColor });

    trk.scrollParamIndex.push_back({ (int)p, 2 });
    trk.scrollParamIndex.push_back({ (int)p + 2, 2 });

    p += 3; // Skip the next 3 parameters (Decay, Sustain, Release)
}

void drawParam(Draw& d, Track& trk, Param* params, size_t& p, const int colW, const int winW, int x, int y, Color& bgColor, const std::chrono::steady_clock::time_point& now)
{
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
    trk.scrollParamIndex.push_back({ p, 1 });
}

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

        trk.scrollParamIndex.clear();
        int visualIdx = 0;
        for (size_t p = 0; p < paramCount; p++, visualIdx++) {
            int x = MARGIN + ((int)visualIdx % paramsPerRow) * colW;
            int y = currentY + ((int)visualIdx / paramsPerRow) * ROW_H;

            bool isADSR = (p + 3 < paramCount) && (params[p].module == MODULE_ENV_ADSR && params[p + 3].module == MODULE_ENV_ADSR); // could add && params[p + 1].module == MODULE_ENV_ADSR && params[p + 2].module == MODULE_ENV_ADSR && but i guess it s enough..
            bool isWavetablePair = (p + 1 < paramCount) && (params[p].module == MODULE_OSC_WAVETABLE) && (params[p + 1].module == MODULE_OSC_WAVETABLE);

            if (isADSR && (visualIdx % paramsPerRow) < (paramsPerRow - 1)) {
                drawADSR(d, trk, params, p, colW, x, y, bgColor);
                visualIdx++; // We used an extra visual slot!
            } else if (isWavetablePair) {
                drawWavetable(d, trk, params, p, colW, x, y, bgColor);
            } else {
                drawParam(d, trk, params, p, colW, winW, x, y, bgColor, now);
            }
        }

        int secH = (((int)trk.engine->getParamCount() + 7) / 8) * ROW_H + TRACK_H;
        trk.trackBounds = sf::IntRect(MARGIN, startY, winW - MARGIN * 2, secH);
        currentY += secH - TRACK_H + 2;
    }

    return currentY;
}

void handelTracksMouseWheelScrolled(sf::RenderWindow& window, sf::Event& event, bool& static_needs_redraw)
{
    int mx = event.mouseWheelScroll.x, my = event.mouseWheelScroll.y;
    float delta = event.mouseWheelScroll.delta;
    uint32_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

    for (auto& trk : studio.tracks) {
        if (trk->volRect.contains(mx, my)) {
            int scaled = encGetScaledDirection(delta, now, trk->lastVolShiftTick);
            trk->lastVolShiftTick = now;
            trk->volume = std::clamp(trk->volume + scaled * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 0.05f : 0.01f), 0.f, 1.f);
            static_needs_redraw = true;
        } else if (trk->trackBounds.contains(mx, my)) {
            const int winW = (int)window.getSize().x;
            const int cW = (winW - MARGIN * 2) / 8;

            int vIdx = ((my - (trk->trackBounds.top + TRACK_H)) / ROW_H) * 8 + (mx - MARGIN) / cW;

            if (vIdx >= 0 && vIdx < trk->scrollParamIndex.size()) {
                int basePIdx = trk->scrollParamIndex[vIdx].first;
                int nParams = trk->scrollParamIndex[vIdx].second; // 1, 2, 3, etc.

                int finalPIdx = basePIdx;

                if (nParams > 1) {
                    int relX = (mx - MARGIN) % cW;
                    int zoneWidth = cW / nParams;
                    int zoneIndex = relX / zoneWidth;
                    if (zoneIndex >= nParams) zoneIndex = nParams - 1;
                    finalPIdx = basePIdx + zoneIndex;
                }

                if (finalPIdx >= 0 && (size_t)finalPIdx < trk->engine->getParamCount()) {
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    Param& p = trk->engine->getParams()[finalPIdx];

                    int scaled = encGetScaledDirection(delta, now, trk->lastShiftTicks[finalPIdx]);
                    trk->lastShiftTicks[finalPIdx] = now;

                    p.set(p.value + scaled * p.step * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.f : 1.f));

                    trk->activeParamIdx = finalPIdx;
                    trk->lastEditTime = std::chrono::steady_clock::now();
                    static_needs_redraw = true;
                }
            }
        }
    }
}