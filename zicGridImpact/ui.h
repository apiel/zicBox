#pragma once

#include "draw/draw.h"
#include "ui/uiParams.h"
#include "zicGridImpact/gridState.h"
#include "zicGridImpact/studio.h"

#include "zicGridImpact/views/MasterView.h"
#include "zicGridImpact/views/SequencerView.h"
#include "zicGridImpact/views/KickView.h"
#include "zicGridImpact/views/Synth1View.h"
#include "zicGridImpact/views/Synth2View.h"

#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"

inline MasterView masterView;
inline SequencerView seqView;
inline KickView kickView;
inline Synth1View synth1View;
inline Synth2View synth2View;

inline void updateActiveViewEncoders() {
    switch (gridState.activeView) {
        case VIEW_MASTER: masterView.updateEncoders(); break;
        case VIEW_SEQUENCER: seqView.updateEncoders(); break;
        case VIEW_KICK: kickView.updateEncoders(); break;
        case VIEW_SYNTH1: synth1View.updateEncoders(); break;
        case VIEW_SYNTH2: synth2View.updateEncoders(); break;
    }
}

inline void handleEncoderInput(int encoderIdx, int delta) {
    if (encoderIdx < 0 || encoderIdx >= TOTAL_ENCODERS) return;
    switch (gridState.activeView) {
        case VIEW_MASTER: masterView.handleEncoder(encoderIdx, delta); break;
        case VIEW_SEQUENCER: seqView.handleEncoder(encoderIdx, delta); break;
        case VIEW_KICK: kickView.handleEncoder(encoderIdx, delta); break;
        case VIEW_SYNTH1: synth1View.handleEncoder(encoderIdx, delta); break;
        case VIEW_SYNTH2: synth2View.handleEncoder(encoderIdx, delta); break;
    }
}

inline void processPerformancePadState() {
    gridState.isPressedA = gridState.pads[0][3].pressed;
    gridState.isPressedS = gridState.pads[1][3].pressed;
    gridState.isPressedZ = gridState.pads[3][3].pressed;
    gridState.isPressedX = gridState.pads[4][3].pressed;
    gridState.isPressedC = gridState.pads[5][3].pressed;
    gridState.isPressedV = gridState.pads[6][3].pressed;

    // Latch processing (Pad 2, Row 3)
    if (gridState.pads[2][3].pressed) {
        if (gridState.isPressedA) gridState.isLatchedA = !gridState.isLatchedA;
        if (gridState.isPressedS) gridState.isLatchedS = !gridState.isLatchedS;
        if (gridState.isPressedZ) gridState.isLatchedZ = !gridState.isLatchedZ;
        if (gridState.isPressedX) gridState.isLatchedX = !gridState.isLatchedX;
        if (gridState.isPressedC) gridState.isLatchedC = !gridState.isLatchedC;
        if (gridState.isPressedV) gridState.isLatchedV = !gridState.isLatchedV;
    }

    studio.kick.isBodyMuted = gridState.isLatchedA || gridState.isPressedA;
    studio.seq.isKickRepeatActive = gridState.isLatchedS || gridState.isPressedS;
    studio.scatter.setModeActive(4, gridState.isLatchedZ || gridState.isPressedZ);
    studio.scatter.setModeActive(5, gridState.isLatchedX || gridState.isPressedX);
    studio.scatter.setModeActive(2, gridState.isLatchedC || gridState.isPressedC);
    studio.scatter.setModeActive(6, gridState.isLatchedV || gridState.isPressedV);
}

inline void handlePadPress(int col, int row, bool pressed) {
    gridState.pads[col][row].pressed = pressed;

    if (pressed) {
        // Row 0: View Navigation & Quick Controls
        if (row == 0) {
            if (col == 0) gridState.activeView = VIEW_MASTER;
            else if (col == 1) gridState.activeView = VIEW_SEQUENCER;
            else if (col == 2) gridState.activeView = VIEW_KICK;
            else if (col == 3) gridState.activeView = VIEW_SYNTH1;
            else if (col == 4) gridState.activeView = VIEW_SYNTH2;
            else if (col == 9) studio.seq.isPlaying = !studio.seq.isPlaying;
            else if (col == 10) gridState.currentOctave = std::max(0, gridState.currentOctave - 1);
            else if (col == 11) gridState.currentOctave = std::min(7, gridState.currentOctave + 1);
            updateActiveViewEncoders();
            return;
        }

        // Row 3: Performance Row
        if (row == 3) {
            if (col == 7) studio.seq.isPlaying = !studio.seq.isPlaying;
            else if (col == 8) gridState.isSynth1Muted = !gridState.isSynth1Muted;
            else if (col == 9) gridState.isSynth2Muted = !gridState.isSynth2Muted;
            else if (col == 10) gridState.currentOctave = std::max(0, gridState.currentOctave - 1);
            else if (col == 11) gridState.currentOctave = std::min(7, gridState.currentOctave + 1);
            processPerformancePadState();
            return;
        }

        // Middle Rows (Rows 1 & 2): Contextual Pads
        if (row == 1 || row == 2) {
            if (gridState.activeView == VIEW_SEQUENCER) {
                int stepIdx = (row - 1) * 12 + col;
                if (stepIdx < 64) {
                    studio.seq.kickPattern[stepIdx] = !studio.seq.kickPattern[stepIdx];
                }
            } else if (gridState.activeView == VIEW_KICK) {
                studio.kick.noteOn(36, 1.0f);
            } else if (gridState.activeView == VIEW_SYNTH1) {
                uint8_t note = static_cast<uint8_t>(gridState.currentOctave * 12 + ((row - 1) * 12 + col));
                studio.synth1.noteOn(note, 0.9f);
            } else if (gridState.activeView == VIEW_SYNTH2) {
                uint8_t note = static_cast<uint8_t>(gridState.currentOctave * 12 + ((row - 1) * 12 + col));
                studio.synth2.noteOn(note, 0.9f);
            }
        }
    } else {
        processPerformancePadState();
    }
}

inline void renderEncoderGrid(Draw& d, int x, int y, int w, int h) {
    d.filledRect({ x, y }, { w, h }, { .color = { 20, 24, 32, 255 } });
    d.rect({ x, y }, { w, h }, { .color = { 50, 60, 80, 255 } });

    int colW = w / 4;
    int rowH = h / 3;

    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 4; ++c) {
            int idx = r * 4 + c;
            int ex = x + c * colW;
            int ey = y + r * rowH;

            d.rect({ ex, ey }, { colW, rowH }, { .color = { 35, 45, 60, 255 } });

            auto& enc = gridState.encoders[idx];
            if (enc.label && enc.label[0] != '\0') {
                d.text({ ex + 6, ey + 4 }, enc.label, 8, { .color = { 180, 195, 220, 255 }, .font = &PoppinsLight_8 });

                std::string valStr;
                if (enc.string) {
                    valStr = enc.string;
                } else {
                    char buf[16];
                    snprintf(buf, sizeof(buf), "%.2f", enc.value);
                    valStr = buf;
                    if (enc.unit) valStr += enc.unit;
                }
                d.text({ ex + 6, ey + rowH - 16 }, valStr, 8, { .color = gridState.encoderColors[idx], .font = &PoppinsLight_8 });

                // Parameter level bar
                float norm = (enc.value - enc.min) / (enc.max - enc.min + 0.0001f);
                norm = std::clamp(norm, 0.0f, 1.0f);
                int barW = static_cast<int>(norm * (colW - 12));
                d.filledRect({ ex + 6, ey + rowH - 6 }, { colW - 12, 3 }, { .color = { 40, 50, 65, 255 } });
                d.filledRect({ ex + 6, ey + rowH - 6 }, { barW, 3 }, { .color = gridState.encoderColors[idx] });
            }
        }
    }
}

inline void renderPadGrid(Draw& d, int x, int y, int w, int h) {
    int colW = w / PAD_COLS;
    int rowH = h / PAD_ROWS;

    // Refresh pad colors and labels
    for (int r = 0; r < PAD_ROWS; ++r) {
        for (int c = 0; c < PAD_COLS; ++c) {
            auto& p = gridState.pads[c][r];
            p.color = { 35, 40, 50, 255 };
            p.label = "";

            // Row 0: Views
            if (r == 0) {
                if (c == 0) { p.label = "Master"; p.color = (gridState.activeView == VIEW_MASTER) ? Color { 255, 215, 0, 255 } : Color { 100, 80, 20, 255 }; }
                else if (c == 1) { p.label = "Seq"; p.color = (gridState.activeView == VIEW_SEQUENCER) ? Color { 60, 220, 100, 255 } : Color { 20, 90, 40, 255 }; }
                else if (c == 2) { p.label = "Kick"; p.color = (gridState.activeView == VIEW_KICK) ? Color { 255, 100, 100, 255 } : Color { 100, 30, 30, 255 }; }
                else if (c == 3) { p.label = "Synth1"; p.color = (gridState.activeView == VIEW_SYNTH1) ? Color { 40, 200, 255, 255 } : Color { 10, 80, 110, 255 }; }
                else if (c == 4) { p.label = "Synth2"; p.color = (gridState.activeView == VIEW_SYNTH2) ? Color { 200, 80, 255, 255 } : Color { 90, 20, 110, 255 }; }
                else if (c >= 5 && c <= 8) { p.label = "P" + std::to_string(c - 4); p.color = { 50, 60, 80, 255 }; }
                else if (c == 9) { p.label = studio.seq.isPlaying ? "PAUSE" : "PLAY"; p.color = studio.seq.isPlaying ? Color { 60, 220, 100, 255 } : Color { 200, 60, 60, 255 }; }
                else if (c == 10) { p.label = "Oct-"; p.color = { 100, 120, 150, 255 }; }
                else if (c == 11) { p.label = "Oct+"; p.color = { 100, 120, 150, 255 }; }
            }

            // Row 3: Performance
            if (r == 3) {
                if (c == 0) { p.label = "KICK"; p.color = (gridState.isLatchedA || gridState.isPressedA) ? Color { 255, 50, 50, 255 } : Color { 120, 30, 30, 255 }; }
                else if (c == 1) { p.label = "REPEAT"; p.color = (gridState.isLatchedS || gridState.isPressedS) ? Color { 255, 215, 0, 255 } : Color { 120, 100, 20, 255 }; }
                else if (c == 2) { p.label = "LATCH"; p.color = gridState.pads[2][3].pressed ? Color { 255, 255, 255, 255 } : Color { 80, 80, 100, 255 }; }
                else if (c == 3) { p.label = "CRUNCH"; p.color = (gridState.isLatchedZ || gridState.isPressedZ) ? Color { 0, 220, 255, 255 } : Color { 10, 90, 110, 255 }; }
                else if (c == 4) { p.label = "DRIVE"; p.color = (gridState.isLatchedX || gridState.isPressedX) ? Color { 100, 120, 255, 255 } : Color { 30, 40, 120, 255 }; }
                else if (c == 5) { p.label = "DIST"; p.color = (gridState.isLatchedC || gridState.isPressedC) ? Color { 255, 80, 180, 255 } : Color { 110, 20, 80, 255 }; }
                else if (c == 6) { p.label = "ACID"; p.color = (gridState.isLatchedV || gridState.isPressedV) ? Color { 60, 220, 100, 255 } : Color { 20, 100, 40, 255 }; }
                else if (c == 7) { p.label = studio.seq.isPlaying ? "STOP" : "PLAY"; p.color = studio.seq.isPlaying ? Color { 60, 220, 100, 255 } : Color { 200, 60, 60, 255 }; }
                else if (c == 8) { p.label = "MUTE S1"; p.color = gridState.isSynth1Muted ? Color { 255, 50, 50, 255 } : Color { 50, 80, 50, 255 }; }
                else if (c == 9) { p.label = "MUTE S2"; p.color = gridState.isSynth2Muted ? Color { 255, 50, 50, 255 } : Color { 50, 80, 50, 255 }; }
                else if (c == 10) { p.label = "OCT-"; p.color = { 100, 120, 150, 255 }; }
                else if (c == 11) { p.label = "OCT+"; p.color = { 100, 120, 150, 255 }; }
            }

            // Draw Pad
            int px = x + c * colW;
            int py = y + r * rowH;

            Color drawCol = p.pressed ? Color { 255, 255, 255, 255 } : p.color;
            d.filledRect({ px + 1, py + 1 }, { colW - 2, rowH - 2 }, { .color = drawCol });
            d.rect({ px + 1, py + 1 }, { colW - 2, rowH - 2 }, { .color = { 60, 70, 90, 255 } });

            if (!p.label.empty()) {
                d.textCentered({ px + colW / 2, py + rowH / 2 - 4 }, p.label, 8,
                    { .color = p.pressed ? Color { 0, 0, 0, 255 } : Color { 240, 240, 245, 255 }, .font = &PoppinsLight_8 });
            }
        }
    }
}

inline bool drawUI(Draw& d, int w, int h, bool& needFullRedraw) {
    updateActiveViewEncoders();

    d.filledRect({ 0, 0 }, { w, h }, { .color = d.styles.colors.background });

    int margin = 4;
    int usableW = w - margin * 2;

    // 1. TOP: 12 Encoder Grid (3 rows x 4 cols)
    int encoderH = 150;
    renderEncoderGrid(d, margin, margin, usableW, encoderH);

    // 2. BOTTOM: 48-Pad Grid (4 rows x 12 cols)
    int padGridH = 180;
    int padGridY = h - padGridH - margin;

    // 3. MIDDLE: Active View Region
    int currentY = margin + encoderH + 4;
    int availableMiddleH = padGridY - currentY - 4;

    switch (gridState.activeView) {
        case VIEW_MASTER: masterView.render(d, margin, currentY, usableW, availableMiddleH); break;
        case VIEW_SEQUENCER: seqView.render(d, margin, currentY, usableW, availableMiddleH); break;
        case VIEW_KICK: kickView.render(d, margin, currentY, usableW, availableMiddleH); break;
        case VIEW_SYNTH1: synth1View.render(d, margin, currentY, usableW, availableMiddleH); break;
        case VIEW_SYNTH2: synth2View.render(d, margin, currentY, usableW, availableMiddleH); break;
    }

    renderPadGrid(d, margin, padGridY, usableW, padGridH);

    return true;
}
