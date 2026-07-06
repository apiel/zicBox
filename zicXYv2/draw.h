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

    if (param.label == nullptr) {
        d.text({ x + 4, y + 10 }, "---", 12, { .color = d.styles.colors.text, .font = &PoppinsLight_12 });
        return;
    }
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
    } else if (param.string != nullptr) {
        int segmentCount = 0;
        if (param.max > param.min && param.step > 0.0f) {
            segmentCount = (int)((param.max - param.min) / param.step) + 1;
        } else if (param.max > param.min) {
            segmentCount = (int)(param.max - param.min) + 1;
        }

        if (segmentCount > 1 && segmentCount <= 25) {
            int currentIndex = (int)((param.value - param.min) / (param.step > 0.0f ? param.step : 1.0f));
            currentIndex = std::max(0, std::min(currentIndex, segmentCount - 1));

            int startX = bX;
            int gap = 3;
            int segW = (bW - (gap * (segmentCount - 1))) / segmentCount;

            d.filledRect({ bX, bY }, { bW, 3 }, { .color = { 50, 50, 50 } }); // background

            for (int segIdx = 0; segIdx < segmentCount; segIdx++) {
                int segX = startX + segIdx * (segW + gap);
                d.filledRect({ segX, bY }, { segW, 3 }, { .color = segIdx == currentIndex ? pColor : Color{ 70, 70, 75 } });
            }
        } else {
            d.filledRect({ bX, bY }, { bW, 3 }, { .color = { 50, 50, 50 } }); // background
            d.filledRect({ bX, bY }, { (int)(bW * pct), 3 }, { .color = pColor });
        }
    } else {
        d.filledRect({ bX, bY }, { bW, 3 }, { .color = { 50, 50, 50 } }); // background
        d.filledRect({ bX, bY }, { (int)(bW * pct), 3 }, { .color = pColor });
    }
}

int params(Draw& d, Param* params, size_t paramCount, int winW, int winH, int paramsTopY, int paramsPerRow, Color& themeColor, uint8_t encodersSelection, uint8_t maxVisibleRows)
{
    int totalParamRows = ((int)paramCount + paramsPerRow - 1) / paramsPerRow;

    // 1. Calculate Scrollbar geometry & Adjust parameters drawing area width
    const int SB_WIDTH = 3; // Scrollbar width
    const int SB_GAP = 1; // Gap between parameters and scrollbar
    int scrollbarX = winW - MARGIN - SB_WIDTH;
    int usableWidth = winW - (MARGIN * 2) - (totalParamRows > maxVisibleRows ? SB_WIDTH + SB_GAP : 0);

    // Recalculate cell width slightly to fit nicely without overlapping the scrollbar
    int adjustedColW = usableWidth / paramsPerRow;

    // Total height of the visible window
    int visibleRows = std::min(totalParamRows, (int)maxVisibleRows);
    int visibleH = visibleRows * UiDraw::ROW_H;

    // Clear the background of the visible area
    d.filledRect({ MARGIN, paramsTopY }, { usableWidth, visibleH }, { .color = d.styles.colors.background });

    // 2. Determine Scrolling Window (Which row index to start rendering from)
    // Static window allocation: keeps active row in view
    static int startRow = 0;
    int activeRow = encodersSelection; // Since paramsPerRow == ENCODER_COUNT

    if (activeRow < startRow) {
        startRow = activeRow;
    } else if (activeRow >= startRow + maxVisibleRows) {
        startRow = activeRow - maxVisibleRows + 1;
    }

    int minX = winW, minY = winH;
    int maxX = 0, maxY = 0;
    bool hasActiveGroup = false;

    // 3. Render Loop (Only iterate parameters inside our scrolling window)
    for (size_t p = 0; p < paramCount; p++) {
        int row = (int)p / paramsPerRow;

        // Skip rendering if row is outside the current sliding scroll view window
        if (row < startRow || row >= startRow + maxVisibleRows) continue;

        int col = (int)p % paramsPerRow;

        // Shift Y up based on our scroll window start row
        int x = MARGIN + col * adjustedColW;
        int y = paramsTopY + (row - startRow) * UiDraw::ROW_H;

        Color bgColor = lighten(d.styles.colors.quaternary, 0.2);
        Color pColor = darken(themeColor, 0.4f);

        bool isActiveGroup = (row == activeRow);
        if (isActiveGroup) {
            bgColor = darken(d.styles.colors.quaternary, 0.1);
            pColor = themeColor;

            hasActiveGroup = true;
            if (x < minX) minX = x;
            if (y < minY) minY = y;
            if (x + adjustedColW - 2 > maxX) maxX = x + adjustedColW - 2;
            if (y + UiDraw::ROW_H - 2 > maxY) maxY = y + UiDraw::ROW_H - 2;
        }

        UiDraw::param(d, params[p], adjustedColW, winW, x, y, bgColor, pColor);
    }

    // Draw active row selection indicator border
    if (hasActiveGroup) {
        d.rect({ minX, minY }, { (maxX - minX), (maxY - minY) }, { .color = { 90, 90, 90 } });
    }

    // 4. Render Scrollbar (Only needed if content overflows the max allowed rows)
    if (totalParamRows > maxVisibleRows) {
        // Draw Track background
        d.filledRect({ scrollbarX, paramsTopY }, { SB_WIDTH, visibleH }, { .color = { 40, 40, 40 } });

        // Calculate proportional Thumb height and vertical position
        int thumbH = (visibleH * maxVisibleRows) / totalParamRows;
        if (thumbH < 8) thumbH = 8; // Keep it graspable/visible even with massive parameter counts

        int maxScrollRowOffset = totalParamRows - maxVisibleRows;
        int remainingTrackH = visibleH - thumbH;
        int thumbY = paramsTopY + (startRow * remainingTrackH / maxScrollRowOffset);

        // Draw Thumb indicator
        d.filledRect({ scrollbarX, thumbY }, { SB_WIDTH, thumbH }, { .color = { 130, 130, 130 } });
    }

    return visibleH;
}
}