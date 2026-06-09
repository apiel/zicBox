#pragma once

#include "zicRack/studio.h"

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

void param(Draw& d, Param& param, const int colW, const int winW, int x, int y, Color& bgColor, Color& pColor, const std::chrono::steady_clock::time_point& now)
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
}