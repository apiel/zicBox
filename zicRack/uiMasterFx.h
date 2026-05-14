#pragma once

#include "draw/utils/inRect.h"
#include "zicRack/studio.h"
namespace MasterFx {

bool needsRedraw = true;

bool filterDragging = false;

Rect filterPadRect;

Rect compRects[4]; // Thresh, Ratio, Attack, Release
Rect compMeterRect;

bool drawStatic(Draw& d, const int winW, bool needFullRedraw, int currentY)
{
    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;

    int padW = 240, padH = 120;
    int padX = winW - padW - 160 - MARGIN * 2; // Shifted left to make room for compressor
    int padY = currentY;
    int pointerSize = 6;
    int halfPointerSize = pointerSize / 2;
    filterPadRect = { padX + halfPointerSize, padY + halfPointerSize, padW - pointerSize, padH - pointerSize };

    d.filledRect({ padX, padY }, { padW, padH }, { .color = { 20, 20, 25 } });
    d.rect({ padX, padY }, { padW, padH }, { .color = { 60, 60, 70 } });
    d.line({ padX + padW / 2, padY }, { padX + padW / 2, padY + padH }, { .color = { 40, 40, 45 } });
    d.text({ padX + 5, padY + 5 }, "MASTER FILTER", 8, { .color = { 0, 180, 255 }, .font = &PoppinsLight_8 });

    float fx = (studio.filter.getCutoff() + 1.0f) * 0.5f;
    float fy = 1.0f - studio.filter.getResonance();
    d.filledRect({ filterPadRect.position.x + (int)(fx * filterPadRect.size.w) - 3, filterPadRect.position.y + (int)(fy * filterPadRect.size.h) - 3 }, { pointerSize, pointerSize }, { .color = { 0, 180, 255 } });

    // Compressor UI (Right side of pad)
    int compX = padX + padW + MARGIN;
    int compW = 150;
    d.text({ compX, padY }, "COMPRESSOR", 8, { .color = { 0, 180, 255 }, .font = &PoppinsLight_8 });

    auto drawParam = [&](int idx, std::string label, float val, float min, float max, std::string unit) {
        int py = padY + 15 + idx * 25;
        Point position = { compX, py };
        Size size = { compW - 30, 20 };
        compRects[idx] = { position, size };
        d.filledRect(position, size, { .color = { 30, 30, 35 } });
        float pct = (val - min) / (max - min);
        d.filledRect({ position.x, position.y + 16 }, { (int)(size.w * pct), 2 }, { .color = { 0, 180, 255 } });
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
    compMeterRect = { { meterX, padY + 15 }, { 10, meterH } };

    return true;
}

int lastCutoffY = -1;
bool drawCompressorMeter(Draw& d)
{
    float grDb = studio.compressor.getGainReductionDb();
    float grPct = std::clamp(-grDb / 20.0f, 0.0f, 1.0f);
    int cutoffY = compMeterRect.size.h - (int)(compMeterRect.size.h * grPct);

    if (cutoffY != lastCutoffY) {
        lastCutoffY = cutoffY;

        d.filledRect(compMeterRect.position, compMeterRect.size, { .color = { 30, 30, 35 } });
        d.filledRect({ compMeterRect.position.x, compMeterRect.position.y + cutoffY }, { compMeterRect.size.w, compMeterRect.size.h - cutoffY }, { .color = { 255, 100, 0 } });
        return true;
    }
    return false;
}

bool draw(Draw& d, const int winW, bool needFullRedraw, int currentY)
{
    bool rendered = false;
    rendered |= drawStatic(d, winW, needFullRedraw, currentY);
    rendered |= drawCompressorMeter(d);
    return rendered;
}

void mouseMoved(Point position)
{
    if (filterDragging) {
        float x = position.x - filterPadRect.position.x;
        float y = position.y - filterPadRect.position.y;
        studio.filter.setCutoff(std::clamp((x / filterPadRect.size.w) * 2.0f - 1.0f, -1.0f, 1.0f));
        studio.filter.setResonance(std::clamp(1.0f - (y / filterPadRect.size.h), 0.0f, 1.0f));
        needsRedraw = true;
    }
}

void mouseButtonPressed(Point position)
{
    if (inRect(filterPadRect, position)) {
        filterDragging = true;
        float x = position.x - filterPadRect.position.x;
        float y = position.y - filterPadRect.position.y;
        studio.filter.setCutoff(std::clamp((x / filterPadRect.size.w) * 2.0f - 1.0f, -1.0f, 1.0f));
        studio.filter.setResonance(std::clamp(1.0f - (y / filterPadRect.size.h), 0.0f, 1.0f));
        needsRedraw = true;
    }
}

void mouseButtonReleased()
{
    filterDragging = false;
}

bool mouseWheelScrolled(Point position, int delta)
{
    for (int i = 0; i < 4; i++) {
        if (inRect(compRects[i], position)) {
            float step = (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) ? 5.0f : 1.0f;
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