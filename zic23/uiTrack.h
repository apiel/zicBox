#pragma once

#include "helpers/enc.h"
#include "zic23/studio.h"
#include "zic23/uiClip.h"
#include "zic23/uiEq.h"

void drawWavetable(Draw& d, Track& trk, Param* params, size_t& p, const int colW, int x, int y, Color& bgColor)
{
    int cellW = colW - 2;
    d.filledRect({ x, y }, { cellW, ROW_H - 2 }, { .color = bgColor });

    d.line({ x + cellW / 2, y }, { x + cellW / 2, y + ROW_H - 2 }, { .color = { 50, 50, 50 } });

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

        int visualIdx = 0;
        for (size_t p = 0; p < paramCount; p++, visualIdx++) {
            int x = MARGIN + ((int)visualIdx % paramsPerRow) * colW;
            int y = currentY + ((int)visualIdx / paramsPerRow) * ROW_H;

            bool isWavetablePair = (p + 1 < paramCount) && (params[p].module == MODULE_OSC_WAVETABLE) && (params[p + 1].module == MODULE_OSC_WAVETABLE) && ((int)p % paramsPerRow != (paramsPerRow - 1));

            if (isWavetablePair) {
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
                int pIdx = trk->scrollParamIndex[vIdx].first;
                // int pIdx = ((my - (trk->trackBounds.top + TRACK_H)) / ROW_H) * 8 + (mx - MARGIN) / cW;
                if (pIdx >= 0 && (size_t)pIdx < trk->engine->getParamCount()) {
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    Param& p = trk->engine->getParams()[pIdx];
                    int scaled = encGetScaledDirection(delta, now, trk->lastShiftTicks[pIdx]);
                    trk->lastShiftTicks[pIdx] = now;
                    p.set(p.value + scaled * p.step * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.f : 1.f));
                    trk->activeParamIdx = pIdx;
                    trk->lastEditTime = std::chrono::steady_clock::now();
                    static_needs_redraw = true;
                }
            }
        }
    }
}