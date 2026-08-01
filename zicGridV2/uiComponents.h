#pragma once

#include "draw/draw.h"
#include "draw/utils/Icon.h"
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

            Color border = pad.active ? Color { 255, 255, 255, 255 } : Color { (uint8_t)(pad.color.r / 2), (uint8_t)(pad.color.g / 2), (uint8_t)(pad.color.b / 2), 255 };
            d.rect({ px + 1, py + 1 }, { padW - 2, padH - 2 }, { .color = border, .thickness = (pad.active ? 2 : 1) });

            if (!pad.label.empty()) {
                Color textCol = getContrastTextColor(bg);
                if (pad.label[0] == '&') {
                    Icon icon(d);
                    int iconSize = 12;
                    icon.render(pad.label, { px + (padW - iconSize) / 2, py + (padH - iconSize) / 2 }, iconSize, textCol);
                } else {
                    d.textCentered({ px + padW / 2, py + padH / 2 - 4 }, pad.label, 8, { .color = textCol, .font = &PoppinsLight_8 });
                }
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
            if (r == 0 && c < MAX_TRACKS) bg = studio.tracks[c]->themeColor;
            else if (r == 1 && (c + 4) < MAX_TRACKS) bg = studio.tracks[c + 4]->themeColor;
            else if (r == 3 && gridState.utility.shiftActive) {
                if (c == 0) bg = { 40, 200, 80, 255 };
                else if (c == 1) bg = { 255, 50, 50, 255 };
                else if (c == 2) bg = { 255, 50, 50, 255 };
                else if (c == 3) bg = { 200, 200, 200, 255 };
            }

            bool isSelected = false;
            if (r == 0 && studio.selTrack == c) isSelected = true;
            else if (r == 1 && studio.selTrack == (c + 4)) isSelected = true;
            else if (r == 2 && c < 3 && gridState.utility.activeView == c) isSelected = true;
            else if (r == 2 && c == 3 && gridState.utility.shiftActive) isSelected = true;
            else if (r == 3 && c == 0 && studio.isPlaying) isSelected = true;
            else if (r == 3 && c == 1 && gridState.utility.shiftActive && gridState.utility.recActive) isSelected = true;
            else if (r == 3 && c == 2 && gridState.utility.shiftActive && studio.masterFx.tape.armed.load()) isSelected = true;

            d.filledRect({ px + 2, py + 2 }, { padW - 4, padH - 4 }, { .color = bg });

            if (isSelected) {
                d.filledRect({ px + 2, py + padH - 3 - 2 }, { padW - 4, 3 }, { .color = { 255, 255, 255, 255 } });
                d.filledCircle({ px + padW - 6, py + 7 }, 2, { .color = { 255, 255, 255, 255 } });
            }

            bool isTrackMuted = false;
            if (r == 0 && c < MAX_TRACKS && studio.tracks[c]->isMuted) isTrackMuted = true;
            else if (r == 1 && (c + 4) < MAX_TRACKS && studio.tracks[c + 4]->isMuted) isTrackMuted = true;

            if (isTrackMuted) {
                Icon icon(d);
                Color muteIconCol = getContrastTextColor(bg);
                icon.render("&icon::mute", { px + 4, py + 4 }, { 10, 10 }, muteIconCol);
            }

            std::string labelToDraw = pad.label;
            if (r == 3 && gridState.utility.shiftActive) {
                if (c == 0) labelToDraw = "&icon::play::filled";
                else if (c == 1) labelToDraw = "&icon::record::filled";
                else if (c == 2) labelToDraw = "&icon::tape";
                else if (c == 3) labelToDraw = "&icon::project";
            }

            if (!labelToDraw.empty()) {
                Color textCol = getContrastTextColor(bg);
                if (labelToDraw[0] == '&') {
                    Icon icon(d);
                    int iconSize = 12;
                    icon.render(labelToDraw, { px + (padW - iconSize) / 2, py + (padH - iconSize) / 2 }, iconSize, textCol);
                } else {
                    d.textCentered({ px + padW / 2, py + padH / 2 - 4 }, labelToDraw, 8, { .color = textCol, .font = &PoppinsLight_8 });
                }
            }
        }
    }
}
