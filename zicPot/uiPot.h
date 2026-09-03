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
            return brain.isPlaying ? "RUNNING [Stop]" : "STOPPED [Start]";
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
        d.filledRect({ 0, 0 }, Size{ width, height }, DrawOptions{ .color = { 18, 20, 26, 255 } });

        // 1. OLED Display Box Simulation (Top Left: 340 x 180 - scaled preview of 64x32 OLED)
        Point oledPos = { 40, 30 };
        Size oledSize = { 340, 180 };
        d.filledRect(oledPos, oledSize, DrawOptions{ .color = { 10, 12, 14, 255 } });
        d.rect(oledPos, oledSize, DrawOptions{ .color = { 0, 180, 255, 255 } });

        // Header: Status & BPM
        char statusStr[64];
        snprintf(statusStr, sizeof(statusStr), "zicPot | %s | %.0f BPM",
                 brain.isPlaying ? "RUNNING" : "STOPPED", brain.bpm);
        d.text({ oledPos.x + 15, oledPos.y + 15 }, statusStr, 12, DrawTextOptions{ .color = { 0, 220, 255, 255 } });

        // Step Dots (Showing first 16 steps or current block of 64 steps)
        for (int i = 0; i < 16; ++i) {
            Point dotPos = { oledPos.x + 15 + i * 19, oledPos.y + 40 };
            bool activeStep = (i < (int)brain.kickSequence.size()) && brain.kickSequence[i].active;
            Color dotCol = activeStep ? Color{ 0, 255, 120, 255 } : Color{ 60, 65, 75, 255 };
            if (brain.isPlaying && (brain.currentStep % 16) == i) {
                dotCol = { 255, 255, 255, 255 }; // Highlight current step
            }
            d.filledRect(dotPos, Size{ 14, 10 }, DrawOptions{ .color = dotCol });
        }

        // Active Menu Item or Pot Overlay inside OLED
        if (potOverlayTimer > 0) {
            std::string potTitle = getPotName((PotIndex)lastMovedPotIndex);
            std::string potVal = getPotFormattedValue((PotIndex)lastMovedPotIndex);

            d.text({ oledPos.x + 15, oledPos.y + 70 }, potTitle, 16, DrawTextOptions{ .color = { 255, 200, 0, 255 } });
            d.text({ oledPos.x + 15, oledPos.y + 105 }, potVal, 20, DrawTextOptions{ .color = { 255, 255, 255, 255 } });
        } else {
            const MenuItem& item = menuItems[currentMenuItem];
            char headerBuf[64];
            snprintf(headerBuf, sizeof(headerBuf), "%d/%d: %s",
                     currentMenuItem + 1, (int)menuItems.size(), item.name.c_str());
            d.text({ oledPos.x + 15, oledPos.y + 70 }, headerBuf, 14, DrawTextOptions{ .color = { 180, 200, 220, 255 } });

            std::string valStr = getFormattedMenuItemValue(item, currentMenuItem);
            if (isEditing) {
                valStr = "> " + valStr + " <";
            }
            d.text({ oledPos.x + 15, oledPos.y + 105 }, valStr, 18, DrawTextOptions{ .color = { 255, 255, 255, 255 } });
        }

        // 2. Rotary Encoder Simulation Controls (Next to OLED: x=420, y=30)
        Point encPos = { 420, 30 };
        Size encSize = { 160, 180 };
        d.filledRect(encPos, encSize, DrawOptions{ .color = { 28, 32, 40, 255 } });
        d.rect(encPos, encSize, DrawOptions{ .color = { 80, 90, 110, 255 } });

        d.text({ encPos.x + 20, encPos.y + 15 }, "U34 ENCODER", 12, DrawTextOptions{ .color = { 220, 220, 220, 255 } });
        d.text({ encPos.x + 15, encPos.y + 40 }, "(D11, D9, D10)", 10, DrawTextOptions{ .color = { 120, 130, 150, 255 } });

        // Encoder Push Button
        Point btnPos = { encPos.x + 25, encPos.y + 75 };
        Size btnSize = { 110, 40 };
        Color btnCol = isEditing ? Color{ 220, 100, 0, 255 } : Color{ 50, 60, 75, 255 };
        d.filledRect(btnPos, btnSize, DrawOptions{ .color = btnCol });
        d.rect(btnPos, btnSize, DrawOptions{ .color = { 200, 200, 200, 255 } });
        d.text({ btnPos.x + 10, btnPos.y + 12 }, isEditing ? "EDITING" : "PRESS/SELECT", 10, DrawTextOptions{ .color = { 255, 255, 255, 255 } });

        d.text({ encPos.x + 15, encPos.y + 135 }, "Scroll / Arrows", 10, DrawTextOptions{ .color = { 140, 150, 165, 255 } });

        // 3. 64-Step Generated Sequence Visualizer Bar (x=40, y=240)
        d.text({ 40, 230 }, "64-STEP KICK GENERATOR SEQUENCE (Generator.h)", 12, DrawTextOptions{ .color = { 200, 210, 230, 255 } });
        for (int i = 0; i < 64; ++i) {
            Point stepBoxPos = { 40 + i * 13, 255 };
            Size stepBoxSize = { 11, 40 };
            bool activeStep = (i < (int)brain.kickSequence.size()) && brain.kickSequence[i].active;
            Color stepCol = activeStep ? Color{ 0, 200, 120, 255 } : Color{ 35, 40, 50, 255 };
            if (brain.isPlaying && brain.currentStep == i) {
                stepCol = { 255, 220, 0, 255 }; // Highlight playing head
            }
            d.filledRect(stepBoxPos, stepBoxSize, DrawOptions{ .color = stepCol });
            d.rect(stepBoxPos, stepBoxSize, DrawOptions{ .color = { 70, 80, 95, 255 } });
        }

        // 4. Potentiometers (RV09 ADC Inputs) - 3 Rows matching physical layout
        d.text({ 40, 315 }, "HARDWARE POTENTIOMETERS (RV09 ADC Inputs - Drag or Scroll to turn)", 12, DrawTextOptions{ .color = { 200, 210, 230, 255 } });

        struct PotLayout {
            PotIndex index;
            const char* pin;
            Point pos;
        };

        PotLayout layouts[NUM_POTS] = {
            // Top Row
            { POT_SUB_FREQ, "A8", { 40, 335 } },
            { POT_CLICK_AMT, "A11", { 240, 335 } },
            { POT_DURATION, "A10", { 440, 335 } },
            // Middle Row
            { POT_VCO_MORPH, "A4", { 40, 425 } },
            { POT_FM_DEPTH, "A5", { 240, 425 } },
            { POT_DRIVE, "A6", { 440, 425 } },
            // Bottom Row
            { POT_RUMBLE_AMT, "A3", { 40, 515 } },
            { POT_RUMBLE_GAP, "A2", { 240, 515 } },
            { POT_BPM, "A0", { 440, 515 } },
            { POT_MASTER_VOL, "A1", { 640, 515 } }
        };

        for (int i = 0; i < NUM_POTS; ++i) {
            const PotLayout& pl = layouts[i];
            Point pPos = pl.pos;
            Size pSize = { 180, 75 };

            d.filledRect(pPos, pSize, DrawOptions{ .color = { 25, 28, 35, 255 } });
            d.rect(pPos, pSize, DrawOptions{ .color = { 60, 70, 85, 255 } });

            char labelBuf[64];
            snprintf(labelBuf, sizeof(labelBuf), "[%s] %s", pl.pin, getPotName(pl.index));
            d.text({ pPos.x + 10, pPos.y + 8 }, labelBuf, 11, DrawTextOptions{ .color = { 0, 200, 255, 255 } });

            // Progress bar showing normalized value
            Point barPos = { pPos.x + 10, pPos.y + 32 };
            Size barSize = { 160, 14 };
            d.filledRect(barPos, barSize, DrawOptions{ .color = { 40, 45, 55, 255 } });

            float valNorm = potValues[pl.index];
            Size fillSize = { (int)(barSize.w * valNorm), barSize.h };
            d.filledRect(barPos, fillSize, DrawOptions{ .color = { 0, 220, 140, 255 } });

            std::string valText = getPotFormattedValue(pl.index);
            d.text({ pPos.x + 10, pPos.y + 50 }, valText, 11, DrawTextOptions{ .color = { 220, 220, 220, 255 } });
        }

        return true;
    }
};
