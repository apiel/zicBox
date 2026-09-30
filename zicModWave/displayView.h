#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

#include "draw/draw.h"
#include "zicApp.h"

inline Color waveMakeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
{
    Color c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
    return c;
}

inline DrawOptions waveDrawOpt(Color color, int thickness = 1)
{
    DrawOptions opt;
    opt.color = color;
    opt.thickness = thickness;
    return opt;
}

inline DrawTextOptions waveTextOpt(Color color, void* font = nullptr)
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
        d.filledRect({ ox, oy }, { NATIVE_W * scale, NATIVE_H * scale }, waveDrawOpt(waveMakeColor(10, 12, 16, 255)));

        // 1. Top Navigation Bar (0..22 px)
        d.filledRect({ ox, oy }, { NATIVE_W * scale, 22 * scale }, waveDrawOpt(waveMakeColor(20, 25, 35, 255)));
        d.text({ ox + 8 * scale, oy + 5 * scale }, "zicModWave", 12 * scale, waveTextOpt(waveMakeColor(0, 220, 255, 255)));

        char statusBuf[64];
        if (app.isExternalClock) {
            snprintf(statusBuf, sizeof(statusBuf), "EXT SYNC  VOL:%d%%", (int)app.engine.masterVol.value);
        } else {
            snprintf(statusBuf, sizeof(statusBuf), "%.0f BPM  VOL:%d%%", app.engine.bpmParam.value, (int)app.engine.masterVol.value);
        }
        d.textRight({ ox + (NATIVE_W - 8) * scale, oy + 5 * scale }, statusBuf, 8 * scale, waveTextOpt(app.isPlaying ? waveMakeColor(0, 255, 140, 255) : waveMakeColor(255, 100, 100, 255)));

        // 2. Main Menu / Pot Takeover View (Full Body Card: y = 32..162)
        d.filledRect({ ox + 10 * scale, oy + 32 * scale }, { 300 * scale, 130 * scale }, 6 * scale, waveDrawOpt(waveMakeColor(18, 22, 30, 255)));
        d.rect({ ox + 10 * scale, oy + 32 * scale }, { 300 * scale, 130 * scale }, 6 * scale, waveDrawOpt(waveMakeColor(50, 65, 85, 255)));

        if (app.potOverlayTimer > 0) {
            app.potOverlayTimer--;

            const char* potName = app.getPotName((PotIndex)app.lastMovedPotIndex);
            d.text({ ox + 24 * scale, oy + 48 * scale }, potName, 16 * scale, waveTextOpt(waveMakeColor(0, 200, 255, 255)));

            char potValBuf[32];
            app.getPotFormattedValue((PotIndex)app.lastMovedPotIndex, potValBuf, sizeof(potValBuf));
            d.textRight({ ox + 296 * scale, oy + 48 * scale }, potValBuf, 16 * scale, waveTextOpt(waveMakeColor(255, 255, 255, 255)));

            // Level Bar
            float valNorm = app.potValues[app.lastMovedPotIndex];
            d.filledRect({ ox + 24 * scale, oy + 105 * scale }, { 272 * scale, 24 * scale }, 4 * scale, waveDrawOpt(waveMakeColor(35, 45, 60, 255)));
            if (valNorm > 0.0f) {
                d.filledRect({ ox + 24 * scale, oy + 105 * scale }, { (int)(272.0f * valNorm) * scale, 24 * scale }, 4 * scale, waveDrawOpt(waveMakeColor(0, 230, 150, 255)));
            }
        } else {
            // Segmented Progress Bar for Menu Items
            int totalItems = ZicApp::NUM_MENU_ITEMS;
            int gap = 4;
            int segWidth = (272 - (totalItems - 1) * gap) / totalItems;
            for (int i = 0; i < totalItems; ++i) {
                int sx = ox + (24 + i * (segWidth + gap)) * scale;
                Color segCol = (i == app.currentMenuItem) ? waveMakeColor(0, 220, 255, 255) : waveMakeColor(45, 55, 70, 255);
                int h = (i == app.currentMenuItem) ? 6 : 3;
                d.filledRect({ sx, oy + 40 * scale }, { segWidth * scale, h * scale }, waveDrawOpt(segCol));
            }

            const char* itemName = app.getMenuItemName(app.currentMenuItem);
            d.text({ ox + 24 * scale, oy + 65 * scale }, itemName, 16 * scale, waveTextOpt(waveMakeColor(200, 210, 225, 255)));

            char itemValBuf[64];
            app.getMenuItemFormattedValue(app.currentMenuItem, itemValBuf, sizeof(itemValBuf));

            if (app.isEditing) {
                // Editing mode highlight badge
                d.filledRect({ ox + 24 * scale, oy + 100 * scale }, { 272 * scale, 45 * scale }, 5 * scale, waveDrawOpt(waveMakeColor(230, 120, 0, 255)));
                d.textCentered({ ox + 160 * scale, oy + 115 * scale }, itemValBuf, 16 * scale, waveTextOpt(waveMakeColor(255, 255, 255, 255)));
            } else {
                d.textCentered({ ox + 160 * scale, oy + 115 * scale }, itemValBuf, 16 * scale, waveTextOpt(waveMakeColor(0, 255, 180, 255)));
            }
        }
    }
};
