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
#include "sequenceBrain.h"

enum PotIndex {
    POT_SUB_FREQ = 0, // A8
    POT_CLICK_AMT,    // A11
    POT_DURATION,     // A10
    POT_VCO_MORPH,    // A4
    POT_FM_DEPTH,     // A5
    POT_DRIVE,        // A6
    POT_RUMBLE_AMT,   // A3
    POT_RUMBLE_GAP,   // A2
    POT_BPM,          // A0
    POT_MASTER_VOL,   // A1
    NUM_POTS = 10
};

typedef void (*UpdateCb)(SequenceBrain& brain);

struct MenuItem {
    std::string name;
    Param* param;  // If non-null, points to Engine Param
    float* varPtr; // If non-null, points to float variable
    float minVal;
    float maxVal;
    float stepVal;
    std::string unit;
    bool isInteger;
    UpdateCb onUpdate = nullptr;
};

class UiPot {
public:
    SequenceBrain& brain;
    DriftKick& kick;

    float masterVolume = 0.8f;
    int currentMenuItem = 0;
    bool isEditing = false;

    // Pot overlay state
    int lastMovedPotIndex = -1;
    int32_t potOverlayTimer = 0; // ms or frames

    // Virtual Potentiometer normalized values [0.0, 1.0]
    float potValues[NUM_POTS] = {
        0.31f, // Sub Freq (52 Hz default)
        0.40f, // Click Amt (40% default)
        0.20f, // Duration (350 ms default)
        0.00f, // VCO Morph (0% default)
        0.35f, // FM Depth (35% default)
        0.35f, // Drive (35% default)
        0.00f, // Rumble Amt (0% default)
        0.28f, // Rumble Gap (120 ms default)
        0.33f, // BPM (120 BPM default)
        0.80f  // Master Vol (80% default)
    };

    std::vector<MenuItem> menuItems;

    UiPot(SequenceBrain& b, DriftKick& k)
        : brain(b)
        , kick(k)
    {
        auto cbRegen = [](SequenceBrain& sb) { sb.regenerateKick(); };

        menuItems = {
            { "PLAY / STOP", nullptr, nullptr, 0.0f, 1.0f, 1.0f, "", true },
            { "BPM", nullptr, &brain.bpm, 60.0f, 240.0f, 1.0f, " BPM", true },
            { "Gen Velocity", nullptr, &brain.genP1, 0.0f, 1.0f, 0.05f, "%", false, cbRegen },
            { "Gen Ghosts", nullptr, &brain.genP2, 0.0f, 1.0f, 0.05f, "%", false, cbRegen },
            { "Gen Rumble", nullptr, &brain.genP3, 0.0f, 1.0f, 0.05f, "%", false, cbRegen },
            { "Sub Freq", &kick.baseFreq, nullptr, 30.0f, 100.0f, 1.0f, " Hz", true },
            { "Click Amt", &kick.clickAmt, nullptr, 0.0f, 100.0f, 1.0f, "%", true },
            { "Duration", &kick.duration, nullptr, 50.0f, 1500.0f, 10.0f, " ms", true },
            { "VCO Morph", &kick.vcoMorph, nullptr, 0.0f, 100.0f, 1.0f, "%", true },
            { "FM Depth", &kick.fmDepth, nullptr, 0.0f, 100.0f, 1.0f, "%", true },
            { "Drive", &kick.drive, nullptr, 0.0f, 100.0f, 1.0f, "%", true },
            { "Rumble", &kick.rumbleAmt, nullptr, 0.0f, 100.0f, 1.0f, "%", true },
            { "Rum Gap", &kick.rumbleGap, nullptr, 10.0f, 400.0f, 5.0f, " ms", true },
            { "Master Vol", nullptr, &masterVolume, 0.0f, 1.0f, 0.05f, "%", false }
        };
    }

    const char* getPotName(PotIndex pot)
    {
        switch (pot) {
            case POT_SUB_FREQ: return "Sub Freq";
            case POT_CLICK_AMT: return "Click Amt";
            case POT_DURATION: return "Duration";
            case POT_VCO_MORPH: return "VCO Morph";
            case POT_FM_DEPTH: return "FM Depth";
            case POT_DRIVE: return "Drive";
            case POT_RUMBLE_AMT: return "Rumble Amt";
            case POT_RUMBLE_GAP: return "Rumble Gap";
            case POT_BPM: return "BPM";
            case POT_MASTER_VOL: return "Master Vol";
            default: return "";
        }
    }

    const char* getShortPotName(PotIndex pot)
    {
        switch (pot) {
            case POT_SUB_FREQ: return "SUB";
            case POT_CLICK_AMT: return "CLCK";
            case POT_DURATION: return "DUR";
            case POT_VCO_MORPH: return "MRPH";
            case POT_FM_DEPTH: return "FM";
            case POT_DRIVE: return "DRV";
            case POT_RUMBLE_AMT: return "RUMB";
            case POT_RUMBLE_GAP: return "GAP";
            case POT_BPM: return "BPM";
            case POT_MASTER_VOL: return "VOL";
            default: return "";
        }
    }

    const char* getShortItemName(int index)
    {
        switch (index) {
            case 0: return "PLAY";
            case 1: return "BPM";
            case 2: return "VEL";
            case 3: return "GHST";
            case 4: return "RMBL";
            case 5: return "SUB";
            case 6: return "CLCK";
            case 7: return "DUR";
            case 8: return "MRPH";
            case 9: return "FM";
            case 10: return "DRV";
            case 11: return "RUMB";
            case 12: return "GAP";
            case 13: return "VOL";
            default: return "";
        }
    }

    std::string getPotFormattedValue(PotIndex pot)
    {
        char buf[32];
        switch (pot) {
            case POT_SUB_FREQ: snprintf(buf, sizeof(buf), "%.0f Hz", kick.baseFreq.value); break;
            case POT_CLICK_AMT: snprintf(buf, sizeof(buf), "%.0f %%", kick.clickAmt.value); break;
            case POT_DURATION: snprintf(buf, sizeof(buf), "%.0f ms", kick.duration.value); break;
            case POT_VCO_MORPH: snprintf(buf, sizeof(buf), "%.0f %%", kick.vcoMorph.value); break;
            case POT_FM_DEPTH: snprintf(buf, sizeof(buf), "%.0f %%", kick.fmDepth.value); break;
            case POT_DRIVE: snprintf(buf, sizeof(buf), "%.0f %%", kick.drive.value); break;
            case POT_RUMBLE_AMT: snprintf(buf, sizeof(buf), "%.0f %%", kick.rumbleAmt.value); break;
            case POT_RUMBLE_GAP: snprintf(buf, sizeof(buf), "%.0f ms", kick.rumbleGap.value); break;
            case POT_BPM: snprintf(buf, sizeof(buf), "%.0f BPM", brain.bpm); break;
            case POT_MASTER_VOL: snprintf(buf, sizeof(buf), "%d %%", (int)std::round(masterVolume * 100.0f)); break;
            default: return "";
        }
        return std::string(buf);
    }

    void applyPotValue(PotIndex pot, float normVal)
    {
        normVal = std::clamp(normVal, 0.0f, 1.0f);
        potValues[pot] = normVal;

        switch (pot) {
            case POT_SUB_FREQ:
                kick.baseFreq.set(kick.baseFreq.min + normVal * (kick.baseFreq.max - kick.baseFreq.min));
                break;
            case POT_CLICK_AMT:
                kick.clickAmt.set(kick.clickAmt.min + normVal * (kick.clickAmt.max - kick.clickAmt.min));
                break;
            case POT_DURATION:
                kick.duration.set(kick.duration.min + normVal * (kick.duration.max - kick.duration.min));
                break;
            case POT_VCO_MORPH:
                kick.vcoMorph.set(kick.vcoMorph.min + normVal * (kick.vcoMorph.max - kick.vcoMorph.min));
                break;
            case POT_FM_DEPTH:
                kick.fmDepth.set(kick.fmDepth.min + normVal * (kick.fmDepth.max - kick.fmDepth.min));
                break;
            case POT_DRIVE:
                kick.drive.set(kick.drive.min + normVal * (kick.drive.max - kick.drive.min));
                break;
            case POT_RUMBLE_AMT:
                kick.rumbleAmt.set(kick.rumbleAmt.min + normVal * (kick.rumbleAmt.max - kick.rumbleAmt.min));
                break;
            case POT_RUMBLE_GAP:
                kick.rumbleGap.set(kick.rumbleGap.min + normVal * (kick.rumbleGap.max - kick.rumbleGap.min));
                break;
            case POT_BPM:
                brain.bpm = 60.0f + normVal * (240.0f - 60.0f);
                break;
            case POT_MASTER_VOL:
                masterVolume = normVal;
                break;
            default:
                break;
        }

        lastMovedPotIndex = (int)pot;
        potOverlayTimer = 1500; // 1.5 seconds overlay
    }

    void handleEncoderTurn(int dir)
    {
        potOverlayTimer = 0; // Clear takeover overlay immediately
        if (isEditing) {
            MenuItem& item = menuItems[currentMenuItem];
            float step = item.stepVal > 0.0f ? item.stepVal : 1.0f;
            if (item.param != nullptr) {
                item.param->set(item.param->value + (dir * step));
            } else if (item.varPtr != nullptr) {
                *item.varPtr = std::clamp(*item.varPtr + (dir * step), item.minVal, item.maxVal);
            }
            if (item.onUpdate != nullptr) {
                item.onUpdate(brain);
            }
        } else {
            currentMenuItem += (dir > 0 ? 1 : -1);
            int total = (int)menuItems.size();
            if (currentMenuItem < 0) currentMenuItem = total - 1;
            if (currentMenuItem >= total) currentMenuItem = 0;
        }
    }

    void handleEncoderClick(const SequenceBrain::MidiTxFunc& txFunc = nullptr)
    {
        potOverlayTimer = 0;
        if (currentMenuItem == 0) { // Play/Stop
            brain.togglePlayStop(txFunc);
        } else {
            isEditing = !isEditing;
        }
    }

    std::string getFormattedMenuItemValue(const MenuItem& item, int index)
    {
        if (index == 0) {
            return brain.isPlaying ? "RUNNING" : "STOPPED";
        }
        char buf[32];
        float val = 0.0f;
        if (item.param != nullptr) {
            val = item.param->value;
        } else if (item.varPtr != nullptr) {
            val = *item.varPtr;
        }

        if (item.isInteger) {
            snprintf(buf, sizeof(buf), "%d%s", (int)std::round(val), item.unit.c_str());
        } else if (item.maxVal <= 1.0f && item.minVal >= 0.0f) {
            snprintf(buf, sizeof(buf), "%d%%", (int)std::round(val * 100.0f));
        } else {
            snprintf(buf, sizeof(buf), "%.1f%s", val, item.unit.c_str());
        }
        return std::string(buf);
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
        // =========================================================================
        Point oledCellPos = { 40, 30 };
        // 32x64 OLED centered in 160x150 cell (Scale 2.2x -> 70px x 140px)
        Point oledPos = { oledCellPos.x + 45, oledCellPos.y + 5 };
        Size oledSize = { 70, 140 };

        d.filledRect(oledPos, oledSize, DrawOptions{ .color = { 0, 0, 0, 255 } });
        d.rect(oledPos, oledSize, DrawOptions{ .color = { 220, 220, 220, 255 } });

        // Native 32x64 OLED Content (Strict Monochrome Black & White Only)
        char idxStr[16];
        snprintf(idxStr, sizeof(idxStr), "%d/%d", currentMenuItem + 1, (int)menuItems.size());
        d.text({ oledPos.x + 4, oledPos.y + 6 }, idxStr, 10, DrawTextOptions{ .color = { 255, 255, 255, 255 } });

        std::string playState = brain.isPlaying ? ">" : "||";
        d.text({ oledPos.x + 52, oledPos.y + 6 }, playState, 10, DrawTextOptions{ .color = { 255, 255, 255, 255 } });

        // Divider Line (Native Y=11)
        d.filledRect({ oledPos.x + 2, oledPos.y + 22 }, Size{ 66, 1 }, DrawOptions{ .color = { 255, 255, 255, 255 } });

        if (potOverlayTimer > 0) {
            std::string potShort = getShortPotName((PotIndex)lastMovedPotIndex);
            std::string potVal = getPotFormattedValue((PotIndex)lastMovedPotIndex);

            d.text({ oledPos.x + 4, oledPos.y + 28 }, potShort, 14, DrawTextOptions{ .color = { 255, 255, 255, 255 } });
            d.text({ oledPos.x + 4, oledPos.y + 55 }, potVal, 11, DrawTextOptions{ .color = { 255, 255, 255, 255 } });

            // Monochrome Knob Fill Bar
            Point barPos = { oledPos.x + 4, oledPos.y + 85 };
            Size barSize = { 62, 10 };
            d.rect(barPos, barSize, DrawOptions{ .color = { 255, 255, 255, 255 } });
            float valNorm = potValues[lastMovedPotIndex];
            d.filledRect({ barPos.x + 1, barPos.y + 1 }, Size{ (int)((barSize.w - 2) * valNorm), barSize.h - 2 }, DrawOptions{ .color = { 255, 255, 255, 255 } });
        } else {
            const MenuItem& item = menuItems[currentMenuItem];
            std::string shortTitle = getShortItemName(currentMenuItem);

            d.text({ oledPos.x + 4, oledPos.y + 28 }, shortTitle, 14, DrawTextOptions{ .color = { 255, 255, 255, 255 } });

            std::string valStr = getFormattedMenuItemValue(item, currentMenuItem);
            if (isEditing) {
                // Monochrome inverted block (white background with black text)
                d.filledRect({ oledPos.x + 3, oledPos.y + 52 }, Size{ 64, 24 }, DrawOptions{ .color = { 255, 255, 255, 255 } });
                d.text({ oledPos.x + 6, oledPos.y + 58 }, valStr, 11, DrawTextOptions{ .color = { 0, 0, 0, 255 } });
            } else {
                d.text({ oledPos.x + 4, oledPos.y + 55 }, valStr, 11, DrawTextOptions{ .color = { 255, 255, 255, 255 } });
            }
        }

        // Bottom divider line inside 32x64 OLED
        d.filledRect({ oledPos.x + 2, oledPos.y + 105 }, Size{ 66, 1 }, DrawOptions{ .color = { 255, 255, 255, 255 } });

        // =========================================================================
        // CELL (Row 0, Col 1): ROTARY ENCODER U34 (Pins D11, D9, D10)
        // =========================================================================
        Point encCellPos = { 220, 30 };
        d.text({ encCellPos.x + 10, encCellPos.y + 10 }, "U34 ENCODER", 12, DrawTextOptions{ .color = { 220, 220, 220, 255 } });
        d.text({ encCellPos.x + 10, encCellPos.y + 30 }, "Pins: D11, D9, D10", 10, DrawTextOptions{ .color = { 120, 140, 160, 255 } });

        Point btnPos = { encCellPos.x + 15, encCellPos.y + 60 };
        Size btnSize = { 130, 50 };
        Color btnCol = isEditing ? Color{ 220, 100, 0, 255 } : Color{ 50, 60, 75, 255 };
        d.filledRect(btnPos, btnSize, DrawOptions{ .color = btnCol });
        d.rect(btnPos, btnSize, DrawOptions{ .color = { 200, 200, 200, 255 } });
        d.text({ btnPos.x + 15, btnPos.y + 18 }, isEditing ? "EDITING" : "PUSH: SELECT", 10, DrawTextOptions{ .color = { 255, 255, 255, 255 } });

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
        float a1ValNorm = potValues[POT_MASTER_VOL];
        d.filledRect(a1BarPos, Size{ (int)(a1BarSize.w * a1ValNorm), a1BarSize.h }, DrawOptions{ .color = { 0, 220, 140, 255 } });

        std::string a1ValText = getPotFormattedValue(POT_MASTER_VOL);
        d.text({ a1CellPos.x + 15, a1CellPos.y + 98 }, a1ValText, 14, DrawTextOptions{ .color = { 220, 220, 220, 255 } });

        // =========================================================================
        // ROWS 1, 2, 3: REMAINING 9 POTENTIOMETERS (Corrected User Grid Layout)
        // Row 1: A10 | A6  | A0
        // Row 2: A11 | A5  | A2
        // Row 3: A8  | A4  | A3
        // =========================================================================
        struct PotCell {
            PotIndex index;
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
            snprintf(labelBuf, sizeof(labelBuf), "[%s] %s", pc.pin, getPotName(pc.index));
            d.text({ pPos.x + 10, pPos.y + 12 }, labelBuf, 11, DrawTextOptions{ .color = { 0, 200, 255, 255 } });

            Point barPos = { pPos.x + 15, pPos.y + 55 };
            Size barSize = { 130, 22 };
            d.filledRect(barPos, barSize, DrawOptions{ .color = { 40, 45, 55, 255 } });

            float valNorm = potValues[pc.index];
            Size fillSize = { (int)(barSize.w * valNorm), barSize.h };
            d.filledRect(barPos, fillSize, DrawOptions{ .color = { 0, 220, 140, 255 } });

            std::string valText = getPotFormattedValue(pc.index);
            d.text({ pPos.x + 15, pPos.y + 98 }, valText, 13, DrawTextOptions{ .color = { 220, 220, 220, 255 } });
        }

        // =========================================================================
        // 64-STEP KICK GENERATOR SEQUENCE VISUALIZER BAR (Bottom: y=705)
        // =========================================================================
        for (int i = 0; i < 64; ++i) {
            Point stepBoxPos = { (int)(40 + i * 8.1f), 710 };
            Size stepBoxSize = { 6, 35 };
            bool activeStep = (i < (int)brain.kickSequence.size()) && brain.kickSequence[i].active;
            Color stepCol = activeStep ? Color{ 0, 200, 120, 255 } : Color{ 35, 40, 50, 255 };
            if (brain.isPlaying && brain.currentStep == i) {
                stepCol = { 255, 220, 0, 255 }; // Highlight playing head
            }
            d.filledRect(stepBoxPos, stepBoxSize, DrawOptions{ .color = stepCol });
            d.rect(stepBoxPos, stepBoxSize, DrawOptions{ .color = { 70, 80, 95, 255 } });
        }

        return true;
    }
};
