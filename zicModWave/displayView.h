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
        d.text({ ox + 8 * scale, oy + 5 * scale }, "zicModWave", 10 * scale, waveTextOpt(waveMakeColor(0, 220, 255, 255)));

        char statusBuf[64];
        if (app.isExternalClock) {
            snprintf(statusBuf, sizeof(statusBuf), "EXT SYNC  VOL:%d%%", (int)app.engine.masterVol.value);
        } else {
            snprintf(statusBuf, sizeof(statusBuf), "%.0f BPM  VOL:%d%%", app.engine.bpmParam.value, (int)app.engine.masterVol.value);
        }
        d.textRight({ ox + (NATIVE_W - 8) * scale, oy + 5 * scale }, statusBuf, 9 * scale, waveTextOpt(app.isPlaying ? waveMakeColor(0, 255, 140, 255) : waveMakeColor(255, 100, 100, 255)));

        // 2. Pot Overlay Takeover or Encoder Menu View
        if (app.potOverlayTimer > 0) {
            app.potOverlayTimer--;

            // Pot Takeover Card (y = 26..75)
            d.filledRect({ ox + 10 * scale, oy + 26 * scale }, { 300 * scale, 50 * scale }, 4 * scale, waveDrawOpt(waveMakeColor(24, 30, 42, 255)));
            d.rect({ ox + 10 * scale, oy + 26 * scale }, { 300 * scale, 50 * scale }, 4 * scale, waveDrawOpt(waveMakeColor(0, 200, 255, 255)));

            const char* potName = app.getPotName((PotIndex)app.lastMovedPotIndex);
            d.text({ ox + 20 * scale, oy + 32 * scale }, potName, 10 * scale, waveTextOpt(waveMakeColor(0, 200, 255, 255)));

            char potValBuf[32];
            app.getPotFormattedValue((PotIndex)app.lastMovedPotIndex, potValBuf, sizeof(potValBuf));
            d.textRight({ ox + 290 * scale, oy + 32 * scale }, potValBuf, 11 * scale, waveTextOpt(waveMakeColor(255, 255, 255, 255)));

            // Level Bar
            float valNorm = app.potValues[app.lastMovedPotIndex];
            d.filledRect({ ox + 20 * scale, oy + 56 * scale }, { 260 * scale, 12 * scale }, 2 * scale, waveDrawOpt(waveMakeColor(40, 50, 65, 255)));
            if (valNorm > 0.0f) {
                d.filledRect({ ox + 20 * scale, oy + 56 * scale }, { (int)(260.0f * valNorm) * scale, 12 * scale }, 2 * scale, waveDrawOpt(waveMakeColor(0, 230, 150, 255)));
            }
        } else {
            // Encoder Menu Header (y = 26..75)
            d.filledRect({ ox + 10 * scale, oy + 26 * scale }, { 300 * scale, 50 * scale }, 4 * scale, waveDrawOpt(waveMakeColor(18, 22, 30, 255)));
            d.rect({ ox + 10 * scale, oy + 26 * scale }, { 300 * scale, 50 * scale }, 4 * scale, waveDrawOpt(waveMakeColor(50, 65, 85, 255)));

            // Top Segmented Progress Bar for 7 Menu Items
            int totalItems = ZicApp::NUM_MENU_ITEMS;
            int gap = 3;
            int segWidth = (280 - (totalItems - 1) * gap) / totalItems;
            for (int i = 0; i < totalItems; ++i) {
                int sx = ox + (20 + i * (segWidth + gap)) * scale;
                Color segCol = (i == app.currentMenuItem) ? waveMakeColor(0, 220, 255, 255) : waveMakeColor(50, 60, 75, 255);
                int h = (i == app.currentMenuItem) ? 4 : 2;
                d.filledRect({ sx, oy + 30 * scale }, { segWidth * scale, h * scale }, waveDrawOpt(segCol));
            }

            const char* itemName = app.getMenuItemName(app.currentMenuItem);
            d.text({ ox + 20 * scale, oy + 42 * scale }, itemName, 11 * scale, waveTextOpt(waveMakeColor(200, 210, 225, 255)));

            char itemValBuf[64];
            app.getMenuItemFormattedValue(app.currentMenuItem, itemValBuf, sizeof(itemValBuf));

            if (app.isEditing) {
                // Editing mode highlight badge
                d.filledRect({ ox + 160 * scale, oy + 40 * scale }, { 140 * scale, 26 * scale }, 3 * scale, waveDrawOpt(waveMakeColor(230, 120, 0, 255)));
                d.textCentered({ ox + 230 * scale, oy + 47 * scale }, itemValBuf, 10 * scale, waveTextOpt(waveMakeColor(255, 255, 255, 255)));
            } else {
                d.textRight({ ox + 290 * scale, oy + 47 * scale }, itemValBuf, 11 * scale, waveTextOpt(waveMakeColor(0, 255, 180, 255)));
            }
        }

        // 3. Real-Time Wavetable & Filter Response Curve (y = 80..165)
        d.filledRect({ ox + 10 * scale, oy + 80 * scale }, { 300 * scale, 85 * scale }, 4 * scale, waveDrawOpt(waveMakeColor(14, 17, 24, 255)));
        d.rect({ ox + 10 * scale, oy + 80 * scale }, { 300 * scale, 85 * scale }, 4 * scale, waveDrawOpt(waveMakeColor(40, 50, 65, 255)));

        // Center baseline
        int midY = oy + 122 * scale;
        d.line({ ox + 12 * scale, midY }, { ox + 308 * scale, midY }, waveDrawOpt(waveMakeColor(30, 38, 50, 255)));

        // Cutoff Frequency Indicator Line
        float cutNorm = app.engine.cutoff.value;
        int cutX = ox + (12 + (int)(296.0f * cutNorm)) * scale;
        d.line({ cutX, oy + 82 * scale }, { cutX, oy + 163 * scale }, waveDrawOpt(waveMakeColor(0, 220, 255, 150), 1));

        // Draw Synthesizer Oscillator Waveform Contour
        for (int px = 0; px < 296; px += 2) {
            float normX = (float)px / 296.0f;
            float sampleVal = app.engine.drawImpl(normX);
            int amp = (int)(sampleVal * 34.0f * scale);
            amp = std::clamp(amp, -38 * scale, 38 * scale);

            Color waveCol = waveMakeColor(0, 255, 180, 220);
            if (app.engine.crushFm.value != 0.0f) {
                waveCol = waveMakeColor(255, 180, 0, 220);
            }
            d.line({ ox + (12 + px) * scale, midY - amp }, { ox + (12 + px) * scale, midY + amp }, waveDrawOpt(waveCol, 1 * scale));
        }

        // Filter Response Envelope Contour (LP / BP / HP)
        float fMorph = app.engine.filterMorph.value;
        float reso = app.engine.resonance.value;
        for (int px = 0; px < 296; px += 4) {
            float xNorm = (float)px / 296.0f;
            float response = 0.0f;

            if (fMorph < 0.5f) { // LP to BP
                float t = fMorph * 2.0f;
                float lpResp = (xNorm <= cutNorm) ? 1.0f : std::exp(-(xNorm - cutNorm) * 6.0f);
                float bpResp = std::exp(-std::abs(xNorm - cutNorm) * 8.0f) * (1.0f + reso * 2.0f);
                response = (1.0f - t) * lpResp + t * bpResp;
            } else { // BP to HP
                float t = (fMorph - 0.5f) * 2.0f;
                float bpResp = std::exp(-std::abs(xNorm - cutNorm) * 8.0f) * (1.0f + reso * 2.0f);
                float hpResp = (xNorm >= cutNorm) ? 1.0f : std::exp(-(cutNorm - xNorm) * 6.0f);
                response = (1.0f - t) * bpResp + t * hpResp;
            }

            int h = (int)(response * 32.0f * scale);
            h = std::clamp(h, 0, 38 * scale);
            d.pixel({ ox + (12 + px) * scale, midY - h }, waveMakeColor(0, 200, 255, 255));
        }
    }
};
