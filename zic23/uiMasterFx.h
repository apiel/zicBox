#pragma once

#include "zic23/studio.h"

sf::IntRect filterPadRect;
bool filterDragging = false;

sf::IntRect compRects[4]; // Thresh, Ratio, Attack, Release
sf::IntRect compMeterRect;

void drawMasterFxUI(Draw& d, sf::Vector2u size, int currentY)
{

    const int winW = (int)size.x;
    int padW = 240, padH = 120;
    int padX = winW - padW - 160 - MARGIN * 2; // Shifted left to make room for compressor
    int padY = currentY;
    filterPadRect = { padX, padY, padW, padH };

    d.filledRect({ padX, padY }, { padW, padH }, { .color = { 20, 20, 25 } });
    d.rect({ padX, padY }, { padW, padH }, { .color = { 60, 60, 70 } });
    d.line({ padX + padW / 2, padY }, { padX + padW / 2, padY + padH }, { .color = { 40, 40, 45 } });
    d.text({ padX + 5, padY + 5 }, "MASTER FILTER", 8, { .color = { 0, 180, 255 }, .font = &PoppinsLight_8 });

    float fx = (studio.filter.getCutoff() + 1.0f) * 0.5f;
    float fy = 1.0f - studio.filter.getResonance();
    d.filledRect({ padX + (int)(fx * padW) - 3, padY + (int)(fy * padH) - 3 }, { 6, 6 }, { .color = { 0, 180, 255 } });

    // Compressor UI (Right side of pad)
    int compX = padX + padW + MARGIN;
    int compW = 150;
    d.text({ compX, padY }, "COMPRESSOR", 8, { .color = { 0, 180, 255 }, .font = &PoppinsLight_8 });

    auto drawParam = [&](int idx, std::string label, float val, float min, float max, std::string unit) {
        int py = padY + 15 + idx * 25;
        compRects[idx] = { compX, py, compW - 30, 20 };
        d.filledRect({ compRects[idx].left, compRects[idx].top }, { compRects[idx].width, compRects[idx].height }, { .color = { 30, 30, 35 } });
        float pct = (val - min) / (max - min);
        d.filledRect({ compRects[idx].left, compRects[idx].top + 16 }, { (int)(compRects[idx].width * pct), 2 }, { .color = { 0, 180, 255 } });
        d.text({ compRects[idx].left + 2, compRects[idx].top + 2 }, label, 8, { .color = { 200, 200, 200 }, .font = &PoppinsLight_8 });
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << val << unit;
        d.textRight({ compRects[idx].left + compRects[idx].width - 2, compRects[idx].top + 2 }, ss.str(), 8, { .color = { 150, 150, 150 }, .font = &PoppinsLight_8 });
    };

    drawParam(0, "Threshold", studio.compressor.threshold, -60.0f, 0.0f, "dB");
    drawParam(1, "Ratio", studio.compressor.ratio, 1.0f, 20.0f, ":1");
    drawParam(2, "Attack", studio.compressor.attack * 1000.f, 1.0f, 100.0f, "ms");
    drawParam(3, "Release", studio.compressor.release * 1000.f, 10.0f, 500.0f, "ms");

    int meterX = compX + compW - 20;
    int meterH = padH - 20;
    compMeterRect = { meterX, padY + 15, 10, meterH };

    d.filledRect({ compMeterRect.left, compMeterRect.top },
        { compMeterRect.width, compMeterRect.height },
        { .color = { 20, 20, 25 } });
}

void updateCompressorMeter(std::vector<sf::Uint8>& pixels, int stride)
{
    float grDb = studio.compressor.getGainReductionDb();
    float grPct = std::clamp(-grDb / 20.0f, 0.0f, 1.0f);
    int cutoffY = compMeterRect.height - (int)(compMeterRect.height * grPct);

    for (int y = 0; y < compMeterRect.height; y++) {
        bool isOrange = (y >= cutoffY); // Draw from cutoff down to the bottom

        for (int x = 0; x < compMeterRect.width; x++) {
            size_t idx = ((compMeterRect.top + y) * stride + compMeterRect.left + x) * 4;

            if (isOrange) {
                pixels[idx] = 255; // R
                pixels[idx + 1] = 100; // G
                pixels[idx + 2] = 0; // B
            } else {
                pixels[idx] = 20; // Background R
                pixels[idx + 1] = 20; // Background G
                pixels[idx + 2] = 25; // Background B
            }
        }
    }
}
