#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "audioEngine.h"
#include "displayView.h"
#include "draw/draw.h"
#include "zicApp.h"

class UiModWave {
public:
    ZicApp app;
    DisplayView displayView;

    UiModWave(float sr = 44100.0f)
        : app(sr)
    {}

    void applyPotValue(PotIndex pot, float normVal)
    {
        app.applyPotValue(pot, normVal);
    }

    void handleEncoderTurn(int dir)
    {
        app.handleEncoderTurn(dir);
    }

    void handleEncoderClick()
    {
        app.handleEncoderClick();
    }

    bool draw(Draw& d, int width, int height, bool& needRedraw)
    {
        // Dark theme background for modular rack module
        d.filledRect({ 0, 0 }, Size{ width, height }, DrawOptions{ .color = { 15, 18, 24, 255 } });

        // Outer chassis border
        d.rect({ 10, 10 }, Size{ width - 20, height - 20 }, DrawOptions{ .color = { 45, 55, 70, 255 }, .thickness = 2 });

        // Module Title Header
        d.text({ 30, 22 }, "zicModWave", 16, DrawTextOptions{ .color = { 0, 220, 255, 255 } });
        d.text({ 180, 26 }, "WAVETABLE & VA SYNTHESIZER MODULE", 9, DrawTextOptions{ .color = { 140, 155, 175, 255 } });

        // =========================================================================
        // TOP SECTION: 1.47-INCH LCD DISPLAY SCREEN (320x172 NATIVE RESOLUTION)
        // Rendered directly to main canvas at offset {30, 50} with 2x scale
        // =========================================================================
        Point dispCellPos = { 30, 50 };
        Size dispCellSize = { 640, 344 };

        // Outer frame of LCD Screen
        d.filledRect(dispCellPos, dispCellSize, DrawOptions{ .color = { 10, 12, 16, 255 } });
        d.rect(dispCellPos, dispCellSize, DrawOptions{ .color = { 200, 200, 200, 255 }, .thickness = 2 });

        // Direct zero-allocation display rendering directly onto master canvas!
        displayView.render(d, app, dispCellPos, 2);

        // =========================================================================
        // ROW 1: 3 POTENTIOMETERS + 1 EMPTY SLOT (x=30, 193, 356, 519)
        // Pot 1: Pitch | Pot 2: Wave | Pot 3: Cutoff | Empty
        // =========================================================================
        struct PotUI {
            PotIndex idx;
            const char* pin;
            Point pos;
        };

        PotUI row1Pots[3] = {
            { POT_PITCH,  "GPIO4", { 30, 415 } },
            { POT_WAVE,   "GPIO5", { 193, 415 } },
            { POT_CUTOFF, "GPIO6", { 356, 415 } }
        };

        for (int i = 0; i < 3; ++i) {
            drawPotCell(d, row1Pots[i].pos, row1Pots[i].idx, row1Pots[i].pin);
        }

        // Empty Slot in Row 1 Col 4
        Point emptyPos = { 519, 415 };
        Size emptySize = { 150, 95 };
        d.filledRect(emptyPos, emptySize, DrawOptions{ .color = { 18, 21, 28, 255 } });
        d.rect(emptyPos, emptySize, DrawOptions{ .color = { 40, 48, 60, 255 } });
        d.text({ emptyPos.x + 42, emptyPos.y + 40 }, "[EMPTY]", 10, DrawTextOptions{ .color = { 80, 95, 115, 255 } });

        // =========================================================================
        // ROW 2: 3 POTENTIOMETERS + 1 PUSH ENCODER
        // Pot 4: Reso | Pot 5: Release | Pot 6: Env Amt | Encoder
        // =========================================================================
        PotUI row2Pots[3] = {
            { POT_RESONANCE, "GPIO7", { 30, 520 } },
            { POT_RELEASE,   "GPIO8", { 193, 520 } },
            { POT_ENV_AMT,   "GPIO9", { 356, 520 } }
        };

        for (int i = 0; i < 3; ++i) {
            drawPotCell(d, row2Pots[i].pos, row2Pots[i].idx, row2Pots[i].pin);
        }

        // Push Encoder Cell (Row 2, Col 4: x=519, y=520)
        Point encPos = { 519, 520 };
        Size encSize = { 150, 95 };

        d.filledRect(encPos, encSize, DrawOptions{ .color = { 22, 26, 34, 255 } });
        d.rect(encPos, encSize, DrawOptions{ .color = { 60, 75, 95, 255 } });

        d.text({ encPos.x + 8, encPos.y + 10 }, "[GPIO43] ENCODER", 10, DrawTextOptions{ .color = { 0, 220, 255, 255 } });

        Point btnPos = { encPos.x + 10, encPos.y + 32 };
        Size btnSize = { 130, 32 };
        Color btnCol = app.isEditing ? Color{ 230, 120, 0, 255 } : Color{ 45, 55, 75, 255 };
        d.filledRect(btnPos, btnSize, DrawOptions{ .color = btnCol });
        d.rect(btnPos, btnSize, DrawOptions{ .color = { 200, 200, 200, 255 } });
        d.text({ btnPos.x + 12, btnPos.y + 10 }, app.isEditing ? "PUSH: EDITING" : "PUSH: SELECT", 9, DrawTextOptions{ .color = { 255, 255, 255, 255 } });

        d.text({ encPos.x + 8, encPos.y + 73 }, "Scroll / Click SW", 8, DrawTextOptions{ .color = { 130, 145, 165, 255 } });

        // =========================================================================
        // ROW 3: 4 POTENTIOMETERS
        // Pot 7: Filt Morph | Pot 8: Crsh / FM | Pot 9: Mod Depth | Pot 10: Mod Speed
        // =========================================================================
        PotUI row3Pots[4] = {
            { POT_FILT_MORPH, "GPIO10", { 30, 625 } },
            { POT_CRUSH_FM,   "GPIO11", { 193, 625 } },
            { POT_MOD_DEPTH,  "GPIO41", { 356, 625 } },
            { POT_MOD_SPEED,  "GPIO42", { 519, 625 } }
        };

        for (int i = 0; i < 4; ++i) {
            drawPotCell(d, row3Pots[i].pos, row3Pots[i].idx, row3Pots[i].pin);
        }

        return true;
    }

private:
    void drawPotCell(Draw& d, Point pos, PotIndex idx, const char* pin)
    {
        Size cellSize = { 150, 95 };
        d.filledRect(pos, cellSize, DrawOptions{ .color = { 22, 26, 34, 255 } });
        d.rect(pos, cellSize, DrawOptions{ .color = { 55, 65, 80, 255 } });

        char labelBuf[64];
        snprintf(labelBuf, sizeof(labelBuf), "[%s] %s", pin, app.getPotName(idx));
        d.text({ pos.x + 8, pos.y + 10 }, labelBuf, 10, DrawTextOptions{ .color = { 0, 220, 255, 255 } });

        Point barPos = { pos.x + 10, pos.y + 38 };
        Size barSize = { 130, 20 };
        d.filledRect(barPos, barSize, DrawOptions{ .color = { 40, 45, 55, 255 } });

        float valNorm = app.potValues[idx];
        Size fillSize = { (int)(barSize.w * valNorm), barSize.h };
        d.filledRect(barPos, fillSize, DrawOptions{ .color = { 0, 220, 140, 255 } });

        char valBuf[32];
        app.getPotFormattedValue(idx, valBuf, sizeof(valBuf));
        d.text({ pos.x + 10, pos.y + 68 }, valBuf, 10, DrawTextOptions{ .color = { 220, 220, 220, 255 } });
    }
};
