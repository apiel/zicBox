#pragma once

#include "draw/draw.h"
#include "ui/uiParams.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include <algorithm>
#include <cmath>

inline void renderEncoderGrid(Draw& d, int x, int y, int w, int h)
{
    int cardW = w / ENCODER_COLS;
    int cardH = h / ENCODER_ROWS;

    for (int r = 0; r < ENCODER_ROWS; ++r) {
        for (int c = 0; c < ENCODER_COLS; ++c) {
            int idx = r * ENCODER_COLS + c;
            int cx = x + c * cardW;
            int cy = y + r * cardH;

            auto& paramObj = gridState.encoders[idx];
            Color bgColor = lighten(d.styles.colors.quaternary, 0.2);
            Color pColor = gridState.encoderColors[idx];

            UiParams::param(d, paramObj, cardW, w, cx, cy, bgColor, pColor);
        }
    }
}

inline void renderDynamicPadMatrix(Draw& d, int x, int y, int w, int h)
{
    d.filledRect({ x, y }, { w, h }, { .color = d.styles.colors.background });

    int padW = w / DYNAMIC_PAD_COLS;
    int padH = h / PAD_ROWS;

    for (int r = 0; r < PAD_ROWS; ++r) {
        for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
            int px = x + c * padW;
            int py = y + r * padH;
            const auto& pad = gridState.pads[c][r];

            Color bg = pad.color;
            if (!pad.pressed && !pad.active) {
                bg.r = bg.r / 4;
                bg.g = bg.g / 4;
                bg.b = bg.b / 4;
            }

            d.filledRect({ px + 1, py + 1 }, { padW - 2, padH - 2 }, { .color = bg });

            Color border = pad.active ? Color{ 255, 255, 255, 255 } : Color{ (uint8_t)(pad.color.r / 2), (uint8_t)(pad.color.g / 2), (uint8_t)(pad.color.b / 2), 255 };
            d.rect({ px + 1, py + 1 }, { padW - 2, padH - 2 }, { .color = border, .thickness = (pad.active ? 2 : 1) });

            if (!pad.label.empty()) {
                d.textCentered({ px + padW / 2, py + padH / 2 - 4 }, pad.label, 8, { .color = { 240, 240, 240, 255 }, .font = &PoppinsLight_8 });
            }
        }
    }
}

inline void renderGlobalUtilityZone(Draw& d, int x, int y, int w, int h)
{
    d.filledRect({ x, y }, { w, h }, { .color = d.styles.colors.background });

    int padW = w / GLOBAL_PAD_COLS;
    int padH = h / PAD_ROWS;

    for (int r = 0; r < PAD_ROWS; ++r) {
        for (int c = 0; c < GLOBAL_PAD_COLS; ++c) {
            int px = x + c * padW;
            int py = y + r * padH;
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

            d.filledRect({ px + 1, py + 1 }, { padW - 2, padH - 2 }, { .color = bg });
            Color border = isSelected ? Color{ 255, 255, 255, 255 } : Color{ (uint8_t)(pad.color.r / 2), (uint8_t)(pad.color.g / 2), (uint8_t)(pad.color.b / 2), 255 };
            d.rect({ px + 1, py + 1 }, { padW - 2, padH - 2 }, { .color = border, .thickness = (isSelected ? 2 : 1) });

            d.textCentered({ px + padW / 2, py + padH / 2 - 4 }, pad.label, 8, { .color = { 240, 240, 240, 255 }, .font = &PoppinsLight_8 });
        }
    }
}
