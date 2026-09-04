#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "audio/engines/DriftKick.h"
#include "draw/draw.h"
#include "zicApp.h"

class UiPot {
public:
    ZicApp app;

    UiPot(SequenceBrain& b, DriftKick& k)
        : app(b, k)
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

        // Label above display
        d.text({ oledCellPos.x + 10, oledCellPos.y + 2 }, "OLED 32x64 (HW 1:1)", 9, DrawTextOptions{ .color = { 120, 140, 160, 255 } });

        // Draw 32x64 native pixels 1:1 from app.displayView.canvas onto emulator screen!
        for (int y = 0; y < 64; y++) {
            for (int x = 0; x < 32; x++) {
                if (app.displayView.canvas.getPixel({ x, y })) {
                    Point pPos = { oledPos.x + x * pixelScale.w, oledPos.y + y * pixelScale.h };
                    d.filledRect(pPos, pixelScale, DrawOptions{ .color = { 255, 255, 255, 255 } });
                }
            }
        }

        // =========================================================================
        // CELL (Row 0, Col 1): ROTARY ENCODER U34 (Pins D11, D9, D10)
        // =========================================================================
        Point encCellPos = { 220, 30 };
        d.text({ encCellPos.x + 10, encCellPos.y + 10 }, "U34 ENCODER", 12, DrawTextOptions{ .color = { 220, 220, 220, 255 } });
        d.text({ encCellPos.x + 10, encCellPos.y + 30 }, "Pins: D11, D9, D10", 10, DrawTextOptions{ .color = { 120, 140, 160, 255 } });

        Point btnPos = { encCellPos.x + 15, encCellPos.y + 60 };
        Size btnSize = { 130, 50 };
        Color btnCol = app.isEditing ? Color{ 220, 100, 0, 255 } : Color{ 50, 60, 75, 255 };
        d.filledRect(btnPos, btnSize, DrawOptions{ .color = btnCol });
        d.rect(btnPos, btnSize, DrawOptions{ .color = { 200, 200, 200, 255 } });
        d.text({ btnPos.x + 15, btnPos.y + 18 }, app.isEditing ? "EDITING" : "PUSH: SELECT", 10, DrawTextOptions{ .color = { 255, 255, 255, 255 } });

        d.text({ encCellPos.x + 10, encCellPos.y + 125 }, "Turn: Scroll / Arrows", 9, DrawTextOptions{ .color = { 130, 140, 155, 255 } });

        // =========================================================================
        // CELL (Row 0, Col 2): POT A1 (MASTER VOLUME)
        // =========================================================================
        Point a1CellPos = { 400, 30 };
        d.text({ a1CellPos.x + 10, a1CellPos.y + 10 }, "[A1] Master Vol", 12, DrawTextOptions{ .color = { 0, 200, 255, 255 } });
        d.text({ a1CellPos.x + 10, a1CellPos.y + 30 }, "(Top Right Cell)", 9, DrawTextOptions{ .color = { 120, 140, 160, 255 } });

        Point a1BarPos = { a1CellPos.x + 15, a1CellPos.y + 60 };
        Size a1BarSize = { 130, 22 };
        d.filledRect(a1BarPos, a1BarSize, DrawOptions{ .color = { 40, 45, 55, 255 } });
        float a1ValNorm = app.potValues[POT_MASTER_VOL];
        d.filledRect(a1BarPos, Size{ (int)(a1BarSize.w * a1ValNorm), a1BarSize.h }, DrawOptions{ .color = { 0, 220, 140, 255 } });

        char a1ValBuf[32];
        app.getPotFormattedValue(POT_MASTER_VOL, a1ValBuf, sizeof(a1ValBuf));
        d.text({ a1CellPos.x + 15, a1CellPos.y + 98 }, a1ValBuf, 14, DrawTextOptions{ .color = { 220, 220, 220, 255 } });

        // =========================================================================
        // ROWS 1, 2, 3: REMAINING 9 POTENTIOMETERS (Corrected User Grid Layout)
        // Row 1: A10 | A6  | A0
        // Row 2: A11 | A5  | A2
        // Row 3: A8  | A4  | A3
        // =========================================================================
        struct PotCell {
            PotIndex idx;
            const char* pin;
            Point pos;
        };

        PotCell potCells[9] = {
            // Row 1 (y=200): A10 | A6 | A0
            { POT_DURATION,  "A10", { 40, 200 } },
            { POT_DRIVE,     "A6",  { 220, 200 } },
            { POT_BPM,       "A0",  { 400, 200 } },

            // Row 2 (y=370): A11 | A5 | A2
            { POT_CLICK_AMT, "A11", { 40, 370 } },
            { POT_FM_DEPTH,  "A5",  { 220, 370 } },
            { POT_RUMBLE_GAP,"A2",  { 400, 370 } },

            // Row 3 (y=540): A8 | A4 | A3
            { POT_SUB_FREQ,  "A8",  { 40, 540 } },
            { POT_VCO_MORPH, "A4",  { 220, 540 } },
            { POT_RUMBLE_AMT,"A3",  { 400, 540 } }
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
