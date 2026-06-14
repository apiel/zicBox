#pragma once

#include "zicXYv2/studio.h"

namespace UiDraw {

static constexpr int ROW_H = 36; // param panel row height

void graph(Draw& d, Param& param, const int colW, int x, int y, Color& bgColor, Color& color)
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
    Color c = color;
    d.lines(points, { .color = c });
    c.a = 50;
    d.filledPolygon(points, { .color = c });
}

void param(Draw& d, Param& param, const int colW, const int winW, int x, int y, Color& bgColor, Color& pColor)
{
    d.filledRect({ x, y }, { colW - 2, ROW_H - 2 }, { .color = bgColor });
    d.text({ x + 4, y + 2 }, param.label, 12, { .color = d.styles.colors.text, .font = &PoppinsLight_12 });

    std::stringstream ss;
    if (param.string) {
        ss << param.string;
    } else {
        ss << std::fixed << std::setprecision(param.precision) << param.value << param.unit;
    }

    d.text({ x + 4, y + 16 }, ss.str(), 8, { .color = { 170, 170, 180 }, .font = &PoppinsLight_8, .maxWidth = colW - 8 });

    float range = param.max - param.min;
    float pct = (param.value - param.min) / (range <= 0 ? 1.f : range);
    int bX = x + 4, bY = y + ROW_H - 8, bW = colW - 10;

    if (param.graph != nullptr) {
        graph(d, param, colW, x, y, bgColor, pColor);
    } else if (param.type & VALUE_CENTERED) {
        int mid = bX + bW / 2;
        int fw = (int)((bW / 2) * (param.value / (param.max == 0 ? 1.0f : param.max)));

        d.filledRect({ bX, bY }, { bW, 3 }, { .color = { 50, 50, 50 } }); // background

        if (fw < 0) d.filledRect({ mid + fw, bY }, { std::abs(fw), 3 }, { .color = pColor });
        else d.filledRect({ mid, bY }, { fw, 3 }, { .color = pColor });

        d.filledRect({ mid, bY - 1 }, { 1, 5 }, { .color = { 100, 100, 100 } });
    } else {
        d.filledRect({ bX, bY }, { bW, 3 }, { .color = { 50, 50, 50 } }); // background
        d.filledRect({ bX, bY }, { (int)(bW * pct), 3 }, { .color = pColor });
    }
}


void params(Draw& d, Param* params, size_t paramCount, int winW, int winH, int colW, int paramsTopY, int paramsPerRow, int& currentY, Color& themeColor, uint8_t encodersSelection)
{

    // Calculate total visual slots needed (rounded up to a full row of 8)
    size_t totalSlots = ((paramCount + (ENCODER_COUNT - 1)) / ENCODER_COUNT) * ENCODER_COUNT;

    int totalParamRows = ((int)totalSlots + paramsPerRow - 1) / paramsPerRow;
    int totalParamH = totalParamRows * UiDraw::ROW_H;
    d.filledRect({ MARGIN, paramsTopY }, { winW - (MARGIN * 2), totalParamH }, { .color = d.styles.colors.background });

    // Variables to capture the edges of the active 2x4 group
    int minX = winW, minY = winH;
    int maxX = 0, maxY = 0;
    bool hasActiveGroup = false;

    for (size_t visualIdx = 0; visualIdx < totalSlots; visualIdx++) {
        int row = (int)visualIdx / paramsPerRow;
        int col = (int)visualIdx % paramsPerRow;

        // --- SORTING MAPPING ---
        int blockRow = row / 2; // Which vertical pair of pages we are on (0 = A/B, 1 = C/D)
        int subRow = row % 2; // Top row (0) or bottom row (1) of the physical 2x4 layout
        int blockSide = col / 4; // Left page (0) or right page (1) in the row
        int subCol = col % 4; // Physical encoder column (0 to 3)

        size_t p = (blockRow * 16) + (blockSide * 8) + (subRow * 4) + subCol;

        if (p >= paramCount) continue;

        int x = MARGIN + col * colW;
        int y = paramsTopY + row * UiDraw::ROW_H;
        currentY = y;

        Color bgColor = lighten(d.styles.colors.quaternary, 0.2);
        Color pColor = darken(themeColor, 0.4f);

        bool isActiveGroup = (int)(p / ENCODER_COUNT) == encodersSelection;
        if (isActiveGroup) {
            bgColor = darken(d.styles.colors.quaternary, 0.1);
            pColor = themeColor;

            // Update bounds for the big rectangle
            hasActiveGroup = true;
            if (x < minX) minX = x;
            if (y < minY) minY = y;
            // colW - 2 and ROW_H - 2 match the inner dimensions inside drawParam
            if (x + colW - 2 > maxX) maxX = x + colW - 2;
            if (y + UiDraw::ROW_H - 2 > maxY) maxY = y + UiDraw::ROW_H - 2;
        }

        UiDraw::param(d, params[p], colW, winW, x, y, bgColor, pColor);
    }

    if (hasActiveGroup) {
        d.rect({ minX, minY }, { (maxX - minX), (maxY - minY) }, { .color = { 90, 90, 90 } });
    }
}
}