#pragma once

#include "draw/draw.h"
#include "draw/utils/Icon.h"
#include "ui/uiParams.h"
#include "zicGridV2/ViewManager.h"
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
            Color defaultBg = lighten(d.styles.colors.quaternary, 0.2);
            Color bgColor = (gridState.encoderBgColors[idx].a != 0) ? gridState.encoderBgColors[idx] : defaultBg;
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
            d.rect({ px + 1, py + 1 }, { padW - 2, padH - 2 }, { .color = border });

            if (pad.selected) {
                d.filledRect({ px + 2, py + padH - 3 - 2 }, { padW - 4, 3 }, { .color = { 255, 255, 255, 255 } });
                d.filledCircle({ px + padW - 6, py + 7 }, 2, { .color = { 255, 255, 255, 255 } });
            }

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
            else if (r == 2 && gridState.utility.shiftActive) {
                if (c == 0) bg = { 35, 45, 60, 255 };
                else if (c == 1) bg = { 40, 160, 220, 255 };
                else if (c == 2) bg = { 40, 200, 80, 255 };
                else if (c == 3) bg = { 200, 200, 200, 255 };
            }
            else if (r == 3 && gridState.utility.shiftActive) {
                if (c == 0) bg = { 40, 200, 80, 255 };
                else if (c == 1) bg = { 255, 50, 50, 255 };
                else if (c == 2) {
                    bool tapeRecording = studio.masterFx.tape.recording.load();
                    bool tapeArmed = studio.masterFx.tape.armed.load();
                    if (tapeRecording) {
                        bg = { 255, 40, 40, 255 };
                    } else if (tapeArmed) {
                        auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                        bool blink = (nowMs / 300) % 2 == 0;
                        bg = blink ? Color { 255, 140, 40, 255 } : Color { 80, 40, 10, 255 };
                    } else {
                        bg = { 180, 100, 30, 255 };
                    }
                } else if (c == 3) bg = { 200, 200, 200, 255 };
            }

            bool isSelected = false;
            if (r == 0 && studio.selTrack == c) isSelected = true;
            else if (r == 1 && studio.selTrack == (c + 4)) isSelected = true;
            else if (r == 2 && !gridState.utility.shiftActive && c < 3 && gridState.utility.activeView == c) isSelected = true;
            else if (r == 2 && c == 3 && gridState.utility.shiftActive) isSelected = true;
            else if (r == 3 && c == 0 && studio.isPlaying && gridState.utility.shiftActive) isSelected = true;
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
            if (r == 2 && gridState.utility.shiftActive) {
                if (c == 0) labelToDraw = "";
                else if (c == 1) labelToDraw = "Reload";
                else if (c == 2) labelToDraw = "Save";
                else if (c == 3) labelToDraw = "Shift";
            }
            else if (r == 3 && gridState.utility.shiftActive) {
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

inline void renderFooterBar(Draw& d, int x, int y, int w, int h, int padMatrixW, int globalUtilityW)
{
    // Sleek dark bar background with clean subtle top border line
    d.filledRect({ x, y }, { w, h }, { .color = Color { 14, 18, 26, 255 } });
    d.line({ x, y }, { x + w, y }, { .color = Color { 35, 45, 60, 255 } });

    Icon icon(d);
    int activeView = ViewManager::getActiveViewIdx();
    bool isShift = gridState.utility.shiftActive;

    // ── 1. RIGHT SIDE: Chain Bracket & Label (ONLY on MasterView when Shift is NOT active) ──
    int utilX = x + padMatrixW;
    int utilW = globalUtilityW;

    if (activeView == VIEW_MASTER && !isShift) {
        Color chainCol = Color { 255, 160, 40, 230 }; // Accent color matching POP, REST, LOOP pads
        std::string chainLabel = "chain";
        int labelW = (int)chainLabel.length() * 6;
        int chainCenterX = utilX + (utilW * 3 / 8); // center under POP, REST, LOOP pads (cols 8..10)
        int labelLeftX = chainCenterX - labelW / 2;
        int labelRightX = chainCenterX + labelW / 2;

        int lineY = y + 7;
        int tickH = 3;

        // Left line segment & tick
        int lineStartX = utilX + 6;
        int lineEndX = labelLeftX - 3;
        if (lineEndX > lineStartX) {
            d.line({ lineStartX, lineY }, { lineEndX, lineY }, { .color = chainCol });
            d.line({ lineStartX, lineY - tickH }, { lineStartX, lineY }, { .color = chainCol });
        }

        // Centered label "chain"
        d.text({ labelLeftX, y + 3 }, chainLabel, 8, { .color = chainCol, .font = &PoppinsLight_8 });

        // Right line segment & tick
        int rLineStartX = labelRightX + 3;
        int rLineEndX = utilX + (utilW * 3 / 4) - 6; // end of 3rd pad column (LOOP)
        if (rLineEndX > rLineStartX) {
            d.line({ rLineStartX, lineY }, { rLineEndX, lineY }, { .color = chainCol });
            d.line({ rLineEndX, lineY - tickH }, { rLineEndX, lineY }, { .color = chainCol });
        }
    }

    // ── 2. LEFT SIDE: Redesigned Premium Status Interface (under pad Matrix) ──
    int curX = x + 3;
    int py = y + 2;
    int pillH = h - 4; // 12px height

    // B. Separate Transport Pill Card
    int transW = 16;
    d.filledRect({ curX, py }, { transW, pillH }, { .color = Color { 22, 28, 40, 220 } });
    d.rect({ curX, py }, { transW, pillH }, { .color = Color { 45, 55, 75, 200 } });

    if (studio.isPlaying) {
        icon.render("&icon::play::filled", { curX + 4, py + 2 }, { 7, 7 }, Color { 40, 220, 100, 255 });
    } else {
        icon.render("&icon::stop::filled", { curX + 4, py + 2 }, { 7, 7 }, Color { 110, 125, 145, 200 });
    }
    curX += transW + 4;

    // C. Separate BPM Pill Card
    std::string bpmValStr = std::to_string((int)studio.bpm);
    int bpmW = (int)bpmValStr.length() * 6 + 28;
    d.filledRect({ curX, py }, { bpmW, pillH }, { .color = Color { 22, 28, 40, 220 } });
    d.rect({ curX, py }, { bpmW, pillH }, { .color = Color { 45, 55, 75, 200 } });

    d.text({ curX + 5, py + 2 }, bpmValStr.c_str(), 8, { .color = Color { 240, 245, 255, 255 }, .font = &PoppinsLight_8 });
    d.text({ curX + 5 + (int)bpmValStr.length() * 6 + 3, py + 2 }, "BPM", 8, { .color = Color { 140, 160, 190, 255 }, .font = &PoppinsLight_8 });
    curX += bpmW + 4;

    // D. Status Badges (REC / TAPE / SHIFT)
    if (gridState.utility.recActive) {
        int recW = 34;
        d.filledRect({ curX, py }, { recW, pillH }, { .color = Color { 120, 20, 20, 220 } });
        d.rect({ curX, py }, { recW, pillH }, { .color = Color { 255, 60, 60, 255 } });
        icon.render("&icon::record::filled", { curX + 4, py + 3 }, { 6, 6 }, Color { 255, 90, 90, 255 });
        d.text({ curX + 13, py + 2 }, "REC", 8, { .color = Color { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });
        curX += recW + 5;
    }

    // Tape recording state tracking & auto-toast on completion
    bool tapeArmed = studio.masterFx.tape.armed.load();
    bool tapeRecording = studio.masterFx.tape.recording.load();

    if (studio.wasTapeArmed && !tapeArmed) {
        studio.wasTapeArmed = false;
        bool dummyRedraw = true;
        UiMessage::show("Tape saved: " + studio.lastRecordedTapeFilename, dummyRedraw, 3000);
    }
    if (tapeArmed) {
        studio.wasTapeArmed = true;
    }

    if (tapeArmed) {
        if (tapeRecording) {
            int tapeW = 64;
            d.filledRect({ curX, py }, { tapeW, pillH }, { .color = Color { 140, 20, 20, 230 } });
            d.rect({ curX, py }, { tapeW, pillH }, { .color = Color { 255, 60, 60, 255 } });
            icon.render("&icon::tape", { curX + 3, py }, 12, Color { 255, 90, 90, 255 });
            d.text({ curX + 16, py + 2 }, "TAPE REC", 8, { .color = Color { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });
            curX += tapeW + 5;
        } else {
            auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
            bool blink = (nowMs / 300) % 2 == 0;
            Color tapeBg = blink ? Color { 110, 50, 10, 220 } : Color { 40, 20, 10, 200 };
            Color tapeBorder = blink ? Color { 255, 150, 40, 255 } : Color { 160, 90, 20, 200 };

            int tapeW = 46;
            d.filledRect({ curX, py }, { tapeW, pillH }, { .color = tapeBg });
            d.rect({ curX, py }, { tapeW, pillH }, { .color = tapeBorder });
            icon.render("&icon::tape", { curX + 3, py }, 12, blink ? Color { 255, 170, 50, 255 } : Color { 160, 100, 30, 255 });
            d.text({ curX + 16, py + 2 }, "ARMED", 8, { .color = blink ? Color { 255, 255, 255, 255 } : Color { 180, 180, 180, 255 }, .font = &PoppinsLight_8 });
            curX += tapeW + 5;
        }
    }

    // Active Track & View Pill Badge
    int activeTrkIdx = studio.selTrack;
    auto& activeTrack = studio.tracks[activeTrkIdx];
    Color trkCol = (activeView == VIEW_MASTER) ? Color { 255, 215, 0, 255 } : ((activeView == VIEW_PROJECT) ? Color { 0, 180, 255, 255 } : (activeTrack ? activeTrack->themeColor : Color { 180, 195, 220, 255 }));
    std::string badgeStr = "";
    if (activeView == VIEW_MASTER) {
        badgeStr = "MASTER";
    } else if (activeView == VIEW_PROJECT) {
        badgeStr = "PROJECT";
    } else if (activeView == VIEW_STEP_SEQ) {
        badgeStr = "T" + std::to_string(activeTrkIdx + 1) + " SEQ";
    } else if (activeView == VIEW_INSTRUMENT) {
        if (activeTrack && activeTrack->currentEngineIdx >= 0 && activeTrack->currentEngineIdx < ENGINE_REGISTRY_COUNT) {
            badgeStr = "T" + std::to_string(activeTrkIdx + 1) + " " + engineRegistry[activeTrack->currentEngineIdx].name;
        } else {
            badgeStr = "T" + std::to_string(activeTrkIdx + 1) + " SYNTH";
        }
    }
    int badgeW = (int)badgeStr.length() * 6 + 10;
    Color bgPill = Color { (uint8_t)(trkCol.r / 6), (uint8_t)(trkCol.g / 6), (uint8_t)(trkCol.b / 6), 200 };
    Color borderPill = Color { (uint8_t)(trkCol.r / 2), (uint8_t)(trkCol.g / 2), (uint8_t)(trkCol.b / 2), 200 };

    d.filledRect({ curX, py }, { badgeW, pillH }, { .color = bgPill });
    d.rect({ curX, py }, { badgeW, pillH }, { .color = borderPill });
    d.text({ curX + 5, py + 2 }, badgeStr, 8, { .color = trkCol, .font = &PoppinsLight_8 });
    curX += badgeW + 6;

    if (isShift) {
        int shiftW = 38;
        d.filledRect({ curX, py }, { shiftW, pillH }, { .color = Color { 100, 90, 10, 220 } });
        d.rect({ curX, py }, { shiftW, pillH }, { .color = Color { 255, 220, 40, 255 } });
        d.text({ curX + 5, py + 2 }, "SHIFT", 8, { .color = Color { 255, 240, 150, 255 }, .font = &PoppinsLight_8 });
        curX += shiftW + 5;
    }
}
