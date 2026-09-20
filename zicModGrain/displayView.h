#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

#include "draw/draw.h"
#include "zicApp.h"

inline Color grainMakeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
{
    Color c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
    return c;
}

inline DrawOptions grainDrawOpt(Color color, int thickness = 1)
{
    DrawOptions opt;
    opt.color = color;
    opt.thickness = thickness;
    return opt;
}

inline DrawTextOptions grainTextOpt(Color color, void* font = nullptr)
{
    DrawTextOptions opt;
    opt.color = color;
    opt.font = font;
    return opt;
}

class DisplayView {
public:
    static constexpr int NATIVE_W = 320;
    static constexpr int NATIVE_H = 172;

    void render(Draw& d, ZicApp& app, Point offset = { 0, 0 }, int scale = 1)
    {
        int ox = offset.x;
        int oy = offset.y;

        // Background of display screen
        d.filledRect({ ox, oy }, { NATIVE_W * scale, NATIVE_H * scale }, grainDrawOpt(grainMakeColor(10, 12, 16, 255)));

        // 1. Top Navigation Bar (0..22 px)
        d.filledRect({ ox, oy }, { NATIVE_W * scale, 22 * scale }, grainDrawOpt(grainMakeColor(20, 25, 35, 255)));
        d.text({ ox + 8 * scale, oy + 5 * scale }, "zicModGrain", 10 * scale, grainTextOpt(grainMakeColor(0, 220, 255, 255)));

        char statusBuf[64];
        if (app.isExternalClock) {
            snprintf(statusBuf, sizeof(statusBuf), "EXT SYNC  VOL:%d%%", (int)app.engine.masterVol.value);
        } else {
            snprintf(statusBuf, sizeof(statusBuf), "%.0f BPM  VOL:%d%%", app.engine.bpmParam.value, (int)app.engine.masterVol.value);
        }
        d.textRight({ ox + (NATIVE_W - 8) * scale, oy + 5 * scale }, statusBuf, 9 * scale, grainTextOpt(app.isPlaying ? grainMakeColor(0, 255, 140, 255) : grainMakeColor(255, 100, 100, 255)));

        // 2. Pot Overlay Takeover or Encoder Menu View
        if (app.potOverlayTimer > 0) {
            app.potOverlayTimer--;

            // Pot Takeover Card (y = 26..75)
            d.filledRect({ ox + 10 * scale, oy + 26 * scale }, { 300 * scale, 50 * scale }, 4 * scale, grainDrawOpt(grainMakeColor(24, 30, 42, 255)));
            d.rect({ ox + 10 * scale, oy + 26 * scale }, { 300 * scale, 50 * scale }, 4 * scale, grainDrawOpt(grainMakeColor(0, 200, 255, 255)));

            const char* potName = app.getPotName((PotIndex)app.lastMovedPotIndex);
            d.text({ ox + 20 * scale, oy + 32 * scale }, potName, 10 * scale, grainTextOpt(grainMakeColor(0, 200, 255, 255)));

            char potValBuf[32];
            app.getPotFormattedValue((PotIndex)app.lastMovedPotIndex, potValBuf, sizeof(potValBuf));
            d.textRight({ ox + 290 * scale, oy + 32 * scale }, potValBuf, 11 * scale, grainTextOpt(grainMakeColor(255, 255, 255, 255)));

            // Level Bar
            float valNorm = app.potValues[app.lastMovedPotIndex];
            d.filledRect({ ox + 20 * scale, oy + 56 * scale }, { 260 * scale, 12 * scale }, 2 * scale, grainDrawOpt(grainMakeColor(40, 50, 65, 255)));
            if (valNorm > 0.0f) {
                d.filledRect({ ox + 20 * scale, oy + 56 * scale }, { (int)(260.0f * valNorm) * scale, 12 * scale }, 2 * scale, grainDrawOpt(grainMakeColor(0, 230, 150, 255)));
            }
        } else {
            // Encoder Menu Header (y = 26..75)
            d.filledRect({ ox + 10 * scale, oy + 26 * scale }, { 300 * scale, 50 * scale }, 4 * scale, grainDrawOpt(grainMakeColor(18, 22, 30, 255)));
            d.rect({ ox + 10 * scale, oy + 26 * scale }, { 300 * scale, 50 * scale }, 4 * scale, grainDrawOpt(grainMakeColor(50, 65, 85, 255)));

            // Top Segmented Progress Bar for 12 Menu Items
            int totalItems = ZicApp::NUM_MENU_ITEMS;
            int gap = 2;
            int segWidth = (280 - (totalItems - 1) * gap) / totalItems;
            for (int i = 0; i < totalItems; ++i) {
                int sx = ox + (20 + i * (segWidth + gap)) * scale;
                Color segCol = (i == app.currentMenuItem) ? grainMakeColor(0, 220, 255, 255) : grainMakeColor(50, 60, 75, 255);
                int h = (i == app.currentMenuItem) ? 4 : 2;
                d.filledRect({ sx, oy + 30 * scale }, { segWidth * scale, h * scale }, grainDrawOpt(segCol));
            }

            const char* itemName = app.getMenuItemName(app.currentMenuItem);
            d.text({ ox + 20 * scale, oy + 42 * scale }, itemName, 11 * scale, grainTextOpt(grainMakeColor(200, 210, 225, 255)));

            char itemValBuf[64];
            app.getMenuItemFormattedValue(app.currentMenuItem, itemValBuf, sizeof(itemValBuf));

            if (app.isEditing) {
                // Editing mode highlight badge
                d.filledRect({ ox + 180 * scale, oy + 40 * scale }, { 120 * scale, 26 * scale }, 3 * scale, grainDrawOpt(grainMakeColor(230, 120, 0, 255)));
                d.textCentered({ ox + 240 * scale, oy + 47 * scale }, itemValBuf, 10 * scale, grainTextOpt(grainMakeColor(255, 255, 255, 255)));
            } else {
                d.textRight({ ox + 290 * scale, oy + 47 * scale }, itemValBuf, 11 * scale, grainTextOpt(grainMakeColor(0, 255, 180, 255)));
            }
        }

        // 3. Real-Time Granular Waveform Display (y = 80..165)
        d.filledRect({ ox + 10 * scale, oy + 80 * scale }, { 300 * scale, 85 * scale }, 4 * scale, grainDrawOpt(grainMakeColor(14, 17, 24, 255)));
        d.rect({ ox + 10 * scale, oy + 80 * scale }, { 300 * scale, 85 * scale }, 4 * scale, grainDrawOpt(grainMakeColor(40, 50, 65, 255)));

        // Crop Bounds Markers (Start % & End %)
        float startPct = app.engine.sampleStart.value * 0.01f;
        float endPct = app.engine.sampleEnd.value * 0.01f;
        int cropStartX = ox + (10 + (int)(300.0f * startPct)) * scale;
        int cropEndX = ox + (10 + (int)(300.0f * endPct)) * scale;

        // Shaded outside regions
        int minCropX = ox + 10 * scale;
        int maxCropX = ox + 310 * scale;

        if (cropStartX > minCropX) {
            d.filledRect({ minCropX, oy + 81 * scale }, { cropStartX - minCropX, 83 * scale }, grainDrawOpt(grainMakeColor(8, 10, 14, 180)));
        }
        if (cropEndX < maxCropX) {
            d.filledRect({ cropEndX, oy + 81 * scale }, { maxCropX - cropEndX, 83 * scale }, grainDrawOpt(grainMakeColor(8, 10, 14, 180)));
        }

        d.line({ cropStartX, oy + 81 * scale }, { cropStartX, oy + 164 * scale }, grainDrawOpt(grainMakeColor(0, 180, 255, 255)));
        d.line({ cropEndX, oy + 81 * scale }, { cropEndX, oy + 164 * scale }, grainDrawOpt(grainMakeColor(0, 180, 255, 255)));

        // Loop Region Highlight
        if (app.engine.loopLength.value > 0.5f) {
            float loopStartPct = app.engine.loopStart.value * 0.01f;
            float loopLenPct = app.engine.loopLength.value * 0.001f;
            int lStartX = cropStartX + (int)((cropEndX - cropStartX) * loopStartPct);
            int lEndX = std::min(cropEndX, lStartX + (int)(300.0f * loopLenPct * scale));
            if (lEndX > lStartX) {
                d.filledRect({ lStartX, oy + 81 * scale }, { lEndX - lStartX, 83 * scale }, grainDrawOpt(grainMakeColor(0, 100, 180, 50)));
                d.line({ lStartX, oy + 81 * scale }, { lStartX, oy + 164 * scale }, grainDrawOpt(grainMakeColor(0, 255, 120, 255), 1));
            }
        }

        // Draw Audio Waveform Contour
        int midY = oy + 122 * scale;
        for (int px = 0; px < 298; px += 2) {
            float normX = (float)px / 298.0f;
            float sampleVal = app.engine.drawImpl(normX);
            int amp = (int)(sampleVal * 35.0f * scale);
            amp = std::clamp(amp, -38 * scale, 38 * scale);
            d.line({ ox + (11 + px) * scale, midY - amp }, { ox + (11 + px) * scale, midY + amp }, grainDrawOpt(grainMakeColor(80, 120, 170, 255)));
        }

        // Draw Active Grain Playhead Indicators
        int activeGrains = app.engine.getVoiceCountImpl();
        for (int g = 0; g < activeGrains; ++g) {
            float ph = app.engine.getPlayheadImpl(g);
            if (ph >= 0.0f && ph <= 1.0f) {
                int gx = ox + (10 + (int)(300.0f * ph)) * scale;
                Color grainDotCol = (g == 0) ? grainMakeColor(255, 220, 0, 255) : grainMakeColor(0, 255, 220, 255);
                d.line({ gx, oy + 81 * scale }, { gx, oy + 164 * scale }, grainDrawOpt(grainDotCol, 2 * scale));
                d.filledCircle({ gx, midY }, 3 * scale, grainDrawOpt(grainMakeColor(255, 255, 255, 255)));
            }
        }
    }
};
