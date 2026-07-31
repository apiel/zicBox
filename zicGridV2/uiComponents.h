#pragma once

#include "draw/draw.h"
#include "zicGridV2/DriftVisualizer.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include <algorithm>
#include <cmath>

class EncoderGridComponent {
public:
    void render(Draw& d, int x, int y, int w, int h)
    {
        d.filledRect({ x, y }, { w, h }, { .color = { 15, 18, 24, 240 } });
        d.rect({ x, y }, { w, h }, { .color = { 45, 55, 75, 255 }, .thickness = 1 });

        int cardW = (w - 8) / ENCODER_COLS;
        int cardH = (h - 6) / ENCODER_ROWS;

        for (int r = 0; r < ENCODER_ROWS; ++r) {
            for (int c = 0; c < ENCODER_COLS; ++c) {
                int idx = r * ENCODER_COLS + c;
                int cx = x + 4 + c * cardW;
                int cy = y + 3 + r * cardH;

                const auto& enc = gridState.encoders[idx];

                // XY / Pixel Drift Param Card style
                Color cardBg = { 38, 42, 52, 230 };
                d.filledRect({ cx + 1, cy + 1 }, { cardW - 2, cardH - 2 }, { .color = cardBg });
                d.rect({ cx + 1, cy + 1 }, { cardW - 2, cardH - 2 }, { .color = { 65, 75, 95, 255 }, .thickness = 1 });

                // Parameter Label (Top Left)
                d.text({ cx + 6, cy + 4 }, enc.label, 10, { .color = { 240, 240, 245, 255 } });

                // Parameter Value Text (Middle / Right)
                d.textRight({ cx + cardW - 6, cy + 18 }, enc.displayVal, 10, { .color = enc.color });

                // Bottom Progress / Segment Bar
                float pct = (enc.value - enc.minVal) / (enc.maxVal > enc.minVal ? (enc.maxVal - enc.minVal) : 1.0f);
                pct = std::clamp(pct, 0.0f, 1.0f);
                int bX = cx + 6;
                int bY = cy + cardH - 8;
                int bW = cardW - 12;

                // Dark progress bar background
                d.filledRect({ bX, bY }, { bW, 4 }, { .color = { 50, 50, 55, 255 } });

                // Active progress bar in theme color
                int fillW = (int)(bW * pct);
                if (fillW > 0) {
                    d.filledRect({ bX, bY }, { fillW, 4 }, { .color = enc.color });
                }
            }
        }
    }
};

class DynamicPadMatrixComponent {
public:
    void render(Draw& d, int x, int y, int w, int h)
    {
        d.filledRect({ x, y }, { w, h }, { .color = { 12, 14, 20, 220 } });
        d.rect({ x, y }, { w, h }, { .color = { 45, 55, 75, 255 }, .thickness = 1 });

        int padW = (w - 10) / DYNAMIC_PAD_COLS;
        int padH = (h - 10) / PAD_ROWS;

        for (int r = 0; r < PAD_ROWS; ++r) {
            for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
                int px = x + 5 + c * padW;
                int py = y + 5 + r * padH;
                const auto& pad = gridState.pads[c][r];

                Color bg = pad.color;
                if (!pad.pressed && !pad.active) {
                    bg.r = bg.r / 4;
                    bg.g = bg.g / 4;
                    bg.b = bg.b / 4;
                }

                d.filledRect({ px + 2, py + 2 }, { padW - 4, padH - 4 }, { .color = bg });

                Color border = pad.active ? Color{ 255, 255, 255, 255 } : Color{ (uint8_t)(pad.color.r / 2), (uint8_t)(pad.color.g / 2), (uint8_t)(pad.color.b / 2), 255 };
                d.rect({ px + 2, py + 2 }, { padW - 4, padH - 4 }, { .color = border, .thickness = (pad.active ? 2 : 1) });

                if (!pad.label.empty()) {
                    d.textCentered({ px + padW / 2, py + padH / 2 - 4 }, pad.label, 9, { .color = { 240, 240, 240, 255 } });
                }
            }
        }
    }
};

class GlobalUtilityZoneComponent {
public:
    void render(Draw& d, int x, int y, int w, int h)
    {
        d.filledRect({ x, y }, { w, h }, { .color = { 16, 18, 26, 240 } });
        d.rect({ x, y }, { w, h }, { .color = { 50, 60, 85, 255 }, .thickness = 1 });

        int padW = (w - 10) / 4;
        int padH = (h - 22) / 4;
        int startY = y + 18;

        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                int px = x + 5 + c * padW;
                int py = startY + r * padH;
                const auto& pad = gridState.pads[8 + c][r];

                Color bg = pad.color;
                if (!pad.pressed) {
                    bg.r = bg.r / 3;
                    bg.g = bg.g / 3;
                    bg.b = bg.b / 3;
                }

                bool isSelected = false;
                if (r == 0 && gridState.utility.activeView == c) isSelected = true;
                if (r == 1 && studio.selTrack == c) isSelected = true;
                if (r == 2 && studio.selTrack == (c + 4)) isSelected = true;
                if (r == 3 && c == 0 && studio.isPlaying) isSelected = true;

                if (isSelected) {
                    bg = pad.color;
                }

                d.filledRect({ px + 2, py + 2 }, { padW - 4, padH - 4 }, { .color = bg });
                Color border = isSelected ? Color{ 255, 255, 255, 255 } : Color{ (uint8_t)(pad.color.r / 2), (uint8_t)(pad.color.g / 2), (uint8_t)(pad.color.b / 2), 255 };
                d.rect({ px + 2, py + 2 }, { padW - 4, padH - 4 }, { .color = border, .thickness = (isSelected ? 2 : 1) });

                d.textCentered({ px + padW / 2, py + padH / 2 - 4 }, pad.label, 9, { .color = { 240, 240, 240, 255 } });
            }
        }
    }
};

class VUVisualizerComponent {
public:
    void render(Draw& d, int x, int y, int w, int h)
    {
        d.filledRect({ x, y }, { w, h }, { .color = { 10, 12, 16, 220 } });
        d.rect({ x, y }, { w, h }, { .color = { 40, 50, 65, 255 }, .thickness = 1 });

        int barW = (w - 18) / MAX_TRACKS;

        for (int i = 0; i < MAX_TRACKS; ++i) {
            int bx = x + 4 + i * (barW + 2);
            float vu = studio.tracks[i]->vumeter.load();
            vu = std::clamp(vu, 0.0f, 1.0f);
            int vuH = (int)((h - 6) * vu);

            Color color = studio.tracks[i]->themeColor;
            if (studio.tracks[i]->isMuted) {
                color = { 80, 80, 80, 255 };
            }

            d.rect({ bx, y + 3 }, { barW, h - 6 }, { .color = { 40, 50, 65, 255 }, .thickness = 1 });
            if (vuH > 0) {
                d.filledRect({ bx + 1, y + 3 + (h - 6 - vuH) }, { barW - 2, vuH }, { .color = color });
            }
        }
    }
};
