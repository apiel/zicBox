#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "audio/engines/KickWave.h"
#include "audio/engines/PotKick.h"
#include "draw/draw.h"
#include "zicApp.h"

class UiModKick {
public:
    ZicApp app;

    UiModKick(SequenceBrain& b, PotKick& pk, KickWave& kw)
        : app(b, pk, kw)
    {}

    // References to ZicApp members for direct access
    float& masterVolume = app.masterVolume;
    float* potValues = app.potValues;
    int32_t& potOverlayTimer = app.potOverlayTimer;

    void applyPotValue(PotIndex pot, float normVal)
    {
        app.applyPotValue(pot, normVal);
    }

    void handleEncoderTurn(int dir)
    {
        app.handleEncoderTurn(dir);
    }

    void handleEncoderClick(const SequenceBrain::MidiTxFunc& txFunc = nullptr)
    {
        app.handleEncoderClick(txFunc);
    }

    // Render SFML desktop UI using Draw interface
    bool draw(Draw& d, int width, int height, bool& needRedraw)
    {
        // Dark theme background
        d.filledRect({ 0, 0 }, Size{ width, height }, DrawOptions{ .color = { 15, 18, 24, 255 } });

        // 4 ROWS x 3 COLUMNS GRID LAYOUT (Each cell represents a 2.5cm x 2.5cm square cell)
        // Cell size: 160px x 160px square
        // Col 0: x=40, Col 1: x=220, Col 2: x=400
        // Row 0: y=30, Row 1: y=200, Row 2: y=370, Row 3: y=540

        // Grid Background borders for the 12 cells (4 rows x 3 cols)
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 3; ++c) {
                Point cellPos = { 40 + c * 180, 30 + r * 170 };
                Size cellSize = { 160, 150 };
                d.filledRect(cellPos, cellSize, DrawOptions{ .color = { 22, 26, 34, 255 } });
                d.rect(cellPos, cellSize, DrawOptions{ .color = { 55, 65, 80, 255 } });
            }
        }

        // =========================================================================
        // CELL (Row 0, Col 0): MONOCHROME OLED DISPLAY ONLY (32x64 pixels native)
        // 1:1 REUSE of ZicApp & DisplayView used by hardware Daisy Seed!
        // =========================================================================
        Point oledCellPos = { 40, 30 };

        // Render exact 32x64 monochrome pixel buffer using shared ZicApp / DisplayView
        app.renderDisplay();

        // Center 32x64 screen inside 160x150 cell (Scale 2x -> 64px wide x 128px high)
        Point oledPos = { oledCellPos.x + 48, oledCellPos.y + 11 };
        Size pixelScale = { 2, 2 };
        Size oledSize = { 32 * pixelScale.w, 64 * pixelScale.h };

        // Outer frame of OLED screen inside cell (0, 0)
        d.filledRect({ oledPos.x - 2, oledPos.y - 2 }, Size{ oledSize.w + 4, oledSize.h + 4 }, DrawOptions{ .color = { 0, 0, 0, 255 } });
        d.rect({ oledPos.x - 2, oledPos.y - 2 }, Size{ oledSize.w + 4, oledSize.h + 4 }, DrawOptions{ .color = { 220, 220, 220, 255 } });

        // Draw 32x64 native pixels 1:1 from app.displayView.canvas onto emulator screen!
        for (int y = 0; y < 64; y++) {
            for (int x = 0; x < 32; x++) {
                if (app.displayView.canvas.getPixel({ x, y })) {
                    Point pPos = { oledPos.x + x * pixelScale.w, oledPos.y + y * pixelScale.h };
                    for (int py = 0; py < pixelScale.h; py++) {
                        for (int px = 0; px < pixelScale.w; px++) {
                            d.pixel({ pPos.x + px, pPos.y + py }, Color{ 255, 255, 255, 255 });
                        }
                    }
                }
            }
        }

        // =========================================================================
        // CELL (Row 0, Col 1): ROTARY ENCODER U34 & 3 BUTTONS (Pins D5, D4, D3)
        // =========================================================================
        Point encCellPos = { 220, 30 };
        d.text({ encCellPos.x + 10, encCellPos.y + 8 }, "ENCODER & BUTTONS", 11, DrawTextOptions{ .color = { 220, 220, 220, 255 } });

        Point btnPos = { encCellPos.x + 10, encCellPos.y + 26 };
        Size btnSize = { 140, 32 };
        Color btnCol = app.isEditing ? Color{ 220, 100, 0, 255 } : Color{ 50, 60, 75, 255 };
        d.filledRect(btnPos, btnSize, DrawOptions{ .color = btnCol });
        d.rect(btnPos, btnSize, DrawOptions{ .color = { 200, 200, 200, 255 } });
        d.text({ btnPos.x + 15, btnPos.y + 10 }, app.isEditing ? "EDITING" : "PUSH: SELECT", 10, DrawTextOptions{ .color = { 255, 255, 255, 255 } });

        // 3 Hardware Buttons (D5, D4, D3)
        // Button 1 [D5]: SHIFT (C)
        Color b1Col = app.isShiftPressed ? Color{ 0, 150, 255, 255 } : Color{ 40, 45, 55, 255 };
        d.filledRect({ encCellPos.x + 10, encCellPos.y + 65 }, Size{ 42, 38 }, DrawOptions{ .color = b1Col });
        d.rect({ encCellPos.x + 10, encCellPos.y + 65 }, Size{ 42, 38 }, DrawOptions{ .color = { 150, 160, 180, 255 } });
        d.text({ encCellPos.x + 12, encCellPos.y + 70 }, "C:D5", 9, DrawTextOptions{ .color = { 255, 255, 255, 255 } });
        d.text({ encCellPos.x + 12, encCellPos.y + 86 }, "SHFT", 8, DrawTextOptions{ .color = { 200, 220, 255, 255 } });

        // Button 2 [D4]: REPEAT / TRANSPOSE (X)
        Color b2Col = app.isTransposeActive ? Color{ 180, 0, 220, 255 } : (app.brain.isNoteRepeatActive ? Color{ 255, 165, 0, 255 } : Color{ 40, 45, 55, 255 });
        d.filledRect({ encCellPos.x + 59, encCellPos.y + 65 }, Size{ 42, 38 }, DrawOptions{ .color = b2Col });
        d.rect({ encCellPos.x + 59, encCellPos.y + 65 }, Size{ 42, 38 }, DrawOptions{ .color = { 150, 160, 180, 255 } });
        d.text({ encCellPos.x + 61, encCellPos.y + 70 }, "X:D4", 9, DrawTextOptions{ .color = { 255, 255, 255, 255 } });
        d.text({ encCellPos.x + 61, encCellPos.y + 86 }, app.isTransposeActive ? "TRSP" : "RPT", 8, DrawTextOptions{ .color = { 255, 220, 180, 255 } });

        // Button 3 [D3]: STOP BODY / TRIG / PLAY (Z)
        Color b3Col = app.isTemporaryBodyMuted ? Color{ 200, 80, 0, 255 } : Color{ 40, 45, 55, 255 };
        d.filledRect({ encCellPos.x + 108, encCellPos.y + 65 }, Size{ 42, 38 }, DrawOptions{ .color = b3Col });
        d.rect({ encCellPos.x + 108, encCellPos.y + 65 }, Size{ 42, 38 }, DrawOptions{ .color = { 150, 160, 180, 255 } });
        d.text({ encCellPos.x + 110, encCellPos.y + 70 }, "Z:D3", 9, DrawTextOptions{ .color = { 255, 255, 255, 255 } });
        d.text({ encCellPos.x + 110, encCellPos.y + 86 }, "BODY", 8, DrawTextOptions{ .color = { 255, 200, 200, 255 } });

        d.text({ encCellPos.x + 10, encCellPos.y + 110 }, "Turn: Scroll / Arrows", 8, DrawTextOptions{ .color = { 130, 140, 155, 255 } });
        d.text({ encCellPos.x + 10, encCellPos.y + 124 }, "C+Z:Play C+X:Trsp Z:Body X:Rpt", 8, DrawTextOptions{ .color = { 110, 160, 200, 255 } });

        // =========================================================================
        // CELL (Row 0, Col 2): POT A1 (DRIVE)
        // =========================================================================
        Point a1CellPos = { 400, 30 };
        d.text({ a1CellPos.x + 10, a1CellPos.y + 10 }, "[A1] Drive", 12, DrawTextOptions{ .color = { 0, 200, 255, 255 } });
        d.text({ a1CellPos.x + 10, a1CellPos.y + 30 }, "(Top Right Cell)", 9, DrawTextOptions{ .color = { 120, 140, 160, 255 } });

        Point a1BarPos = { a1CellPos.x + 15, a1CellPos.y + 60 };
        Size a1BarSize = { 130, 22 };
        d.filledRect(a1BarPos, a1BarSize, DrawOptions{ .color = { 40, 45, 55, 255 } });
        float a1ValNorm = app.potValues[POT_DRIVE];
        d.filledRect(a1BarPos, Size{ (int)(a1BarSize.w * a1ValNorm), a1BarSize.h }, DrawOptions{ .color = { 0, 220, 140, 255 } });

        char a1ValBuf[32];
        app.getPotFormattedValue(POT_DRIVE, a1ValBuf, sizeof(a1ValBuf));
        d.text({ a1CellPos.x + 15, a1CellPos.y + 98 }, a1ValBuf, 14, DrawTextOptions{ .color = { 220, 220, 220, 255 } });

        // =========================================================================
        // ROWS 1, 2, 3: REMAINING 9 POTENTIOMETERS (User Grid Layout)
        // Row 1: A10 | A6 | A0
        // Row 2: A11 | A5 | A2
        // Row 3: A8  | A4 | A3
        // =========================================================================
        struct PotCell {
            PotIndex idx;
            const char* pin;
            Point pos;
        };

        PotCell potCells[9] = {
            // Row 1 (y=200): A10 | A6 | A0
            { POT_FM_DEPTH,    "A10", { 40, 200 } },
            { POT_WAVE_SHAPE,  "A6",  { 220, 200 } },
            { POT_PHASE_OFFSET,"A0",  { 400, 200 } },

            // Row 2 (y=370): A11 | A5 | A2
            { POT_HARMONIC3,   "A11", { 40, 370 } },
            { POT_SKEW,        "A5",  { 220, 370 } },
            { POT_CRUSH,       "A2",  { 400, 370 } },

            // Row 3 (y=540): A8 | A4 | A3
            { POT_FOLD,        "A8",  { 40, 540 } },
            { POT_HARMONIC2,   "A4",  { 220, 540 } },
            { POT_RESONATOR,   "A3",  { 400, 540 } }
        };

        for (int i = 0; i < 9; ++i) {
            const PotCell& pc = potCells[i];
            Point pPos = pc.pos;

            char labelBuf[64];
            snprintf(labelBuf, sizeof(labelBuf), "[%s] %s", pc.pin, app.getPotName(pc.idx));
            d.text({ pPos.x + 10, pPos.y + 12 }, labelBuf, 11, DrawTextOptions{ .color = { 0, 200, 255, 255 } });

            Point barPos = { pPos.x + 15, pPos.y + 55 };
            Size barSize = { 130, 22 };
            d.filledRect(barPos, barSize, DrawOptions{ .color = { 40, 45, 55, 255 } });

            float valNorm = app.potValues[pc.idx];
            Size fillSize = { (int)(barSize.w * valNorm), barSize.h };
            d.filledRect(barPos, fillSize, DrawOptions{ .color = { 0, 220, 140, 255 } });

            char valBuf[32];
            app.getPotFormattedValue(pc.idx, valBuf, sizeof(valBuf));
            d.text({ pPos.x + 15, pPos.y + 98 }, valBuf, 13, DrawTextOptions{ .color = { 220, 220, 220, 255 } });
        }

        // =========================================================================
        // 64-STEP KICK GENERATOR SEQUENCE VISUALIZER BAR (Bottom: y=705)
        // =========================================================================
        for (int i = 0; i < 64; ++i) {
            Point stepBoxPos = { (int)(40 + i * 8.1f), 710 };
            Size stepBoxSize = { 6, 35 };
            bool activeStep = (i < (int)app.brain.kickSequence.size()) && app.brain.kickSequence[i].active;
            Color stepCol = activeStep ? Color{ 0, 200, 120, 255 } : Color{ 35, 40, 50, 255 };
            if (app.brain.isPlaying && app.brain.currentStep == i) {
                stepCol = { 255, 220, 0, 255 }; // Highlight playing head
            }
            d.filledRect(stepBoxPos, stepBoxSize, DrawOptions{ .color = stepCol });
            d.rect(stepBoxPos, stepBoxSize, DrawOptions{ .color = { 70, 80, 95, 255 } });
        }

        return true;
    }
};
