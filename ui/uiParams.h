#pragma once

#include "draw/draw.h"
#include "audio/engines/EngineParam.h"
#include "draw/utils/color.h"
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace UiParams {

static constexpr int ROW_H = 36; // param panel row height
static constexpr int MARGIN = 4; // margin

struct Style {
    Color labelColor = { 0, 0, 0, 0 }; // If alpha == 0, uses d.styles.colors.text
    Color valueColor = { 170, 170, 180, 255 };
    Color barBgColor = { 50, 50, 50, 255 };
    Color inactiveSegColor = { 70, 70, 75, 255 };
    Color midLineColor = { 100, 100, 100, 255 };
    Color borderColor = { 0, 0, 0, 0 }; // If alpha > 0, draws surrounding border rect
};

inline void graph(Draw& d, Param& param, const int colW, int x, int y, Color& bgColor, Color& color)
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

inline void param(Draw& d, Param& param, const int colW, const int winW, int x, int y, Color& bgColor, Color& pColor, Style style = {})
{
    d.filledRect({ x, y }, { colW - 2, ROW_H - 2 }, { .color = bgColor });

    Color lblCol = (style.labelColor.a == 0) ? d.styles.colors.text : style.labelColor;

    if (param.label == nullptr) {
        d.text({ x + 4, y + 10 }, "---", 8, { .color = lblCol, .font = &PoppinsLight_8 });
        if (style.borderColor.a > 0) {
            d.rect({ x, y }, { colW - 2, ROW_H - 2 }, { .color = style.borderColor });
        }
        return;
    }
    d.text({ x + 4, y + 2 }, param.label, 8, { .color = lblCol, .font = &PoppinsLight_8 });

    std::stringstream ss;
    if (param.string) {
        ss << param.string;
    } else {
        int prec = param.precision < 0 ? 2 : param.precision;
        ss << std::fixed << std::setprecision(prec) << param.value << (param.unit ? param.unit : "");
    }

    d.text({ x + 4, y + 16 }, ss.str(), 8, { .color = style.valueColor, .font = &PoppinsLight_8, .maxWidth = colW - 8 });

    float range = param.max - param.min;
    float pct = (param.value - param.min) / (range <= 0.f ? 1.f : range);
    int bX = x + 4, bY = y + ROW_H - 8, bW = colW - 10;

    if (param.graph != nullptr) {
        graph(d, param, colW, x, y, bgColor, pColor);
    } else if (param.type & VALUE_CENTERED) {
        int mid = bX + bW / 2;
        int fw = (int)((bW / 2) * (param.value / (param.max == 0 ? 1.0f : param.max)));

        d.filledRect({ bX, bY }, { bW, 3 }, { .color = style.barBgColor }); // background

        if (fw < 0) d.filledRect({ mid + fw, bY }, { std::abs(fw), 3 }, { .color = pColor });
        else d.filledRect({ mid, bY }, { fw, 3 }, { .color = pColor });

        d.filledRect({ mid, bY - 1 }, { 1, 5 }, { .color = style.midLineColor });
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

            d.filledRect({ bX, bY }, { bW, 3 }, { .color = style.barBgColor }); // background

            for (int segIdx = 0; segIdx < segmentCount; segIdx++) {
                int segX = startX + segIdx * (segW + gap);
                d.filledRect({ segX, bY }, { segW, 3 }, { .color = segIdx == currentIndex ? pColor : style.inactiveSegColor });
            }
        } else {
            d.filledRect({ bX, bY }, { bW, 3 }, { .color = style.barBgColor }); // background
            d.filledRect({ bX, bY }, { (int)(bW * pct), 3 }, { .color = pColor });
        }
    } else {
        d.filledRect({ bX, bY }, { bW, 3 }, { .color = style.barBgColor }); // background
        d.filledRect({ bX, bY }, { (int)(bW * pct), 3 }, { .color = pColor });
    }

    if (style.borderColor.a > 0) {
        d.rect({ x, y }, { colW - 2, ROW_H - 2 }, { .color = style.borderColor });
    }
}

inline int params(Draw& d, Param* params, size_t paramCount, int winW, int winH, int paramsTopY, int paramsPerRow, Color& themeColor, int& startRow, uint8_t encodersSelection, uint8_t maxVisibleRows, Style style = {})
{
    int totalParamRows = ((int)paramCount + paramsPerRow - 1) / paramsPerRow;

    const int SB_WIDTH = 3;
    const int SB_GAP = 1;
    int scrollbarX = winW - MARGIN - SB_WIDTH;
    int usableWidth = winW - (MARGIN * 2) - (totalParamRows > maxVisibleRows ? SB_WIDTH + SB_GAP : 0);

    int adjustedColW = usableWidth / paramsPerRow;
    int visibleRows = std::min(totalParamRows, (int)maxVisibleRows);
    int visibleH = visibleRows * UiParams::ROW_H;

    d.filledRect({ MARGIN, paramsTopY }, { usableWidth, visibleH }, { .color = d.styles.colors.background });

    if (encodersSelection < startRow) {
        startRow = encodersSelection;
    } else if (encodersSelection >= startRow + maxVisibleRows) {
        startRow = encodersSelection - maxVisibleRows + 1;
    }

    int minX = winW, minY = winH;
    int maxX = 0, maxY = 0;
    bool hasActiveGroup = false;

    for (size_t p = 0; p < paramCount; p++) {
        int row = (int)p / paramsPerRow;
        if (row < startRow || row >= startRow + maxVisibleRows) continue;

        int col = (int)p % paramsPerRow;
        int x = MARGIN + col * adjustedColW;
        int y = paramsTopY + (row - startRow) * UiParams::ROW_H;

        Color bgColor = lighten(d.styles.colors.quaternary, 0.2);
        Color pColor = darken(themeColor, 0.4f);

        bool isActiveGroup = (row == encodersSelection);
        if (isActiveGroup) {
            bgColor = darken(d.styles.colors.quaternary, 0.1);
            pColor = themeColor;

            hasActiveGroup = true;
            if (x < minX) minX = x;
            if (y < minY) minY = y;
            if (x + adjustedColW - 2 > maxX) maxX = x + adjustedColW - 2;
            if (y + UiParams::ROW_H - 2 > maxY) maxY = y + UiParams::ROW_H - 2;
        }

        UiParams::param(d, params[p], adjustedColW, winW, x, y, bgColor, pColor, style);
    }

    if (hasActiveGroup) {
        d.rect({ minX, minY }, { (maxX - minX), (maxY - minY) }, { .color = { 90, 90, 90 } });
    }

    if (totalParamRows > maxVisibleRows) {
        d.filledRect({ scrollbarX, paramsTopY }, { SB_WIDTH, visibleH }, { .color = { 40, 40, 40 } });
        int thumbH = (visibleH * maxVisibleRows) / totalParamRows;
        if (thumbH < 8) thumbH = 8;

        int maxScrollRowOffset = totalParamRows - maxVisibleRows;
        int remainingTrackH = visibleH - thumbH;
        int thumbY = paramsTopY + (startRow * remainingTrackH / maxScrollRowOffset);

        d.filledRect({ scrollbarX, thumbY }, { SB_WIDTH, thumbH }, { .color = { 130, 130, 130 } });
    }

    return visibleH;
}

} // namespace UiParams

namespace UiDraw = UiParams;
