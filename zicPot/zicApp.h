#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "audio/engines/DriftKick.h"
#include "displayView.h"
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
    const char* name;
    Param* param;  // If non-null, points to Engine Param
    float* varPtr; // If non-null, points to float variable
    float minVal;
    float maxVal;
    float stepVal;
    const char* unit;
    bool isInteger;
    UpdateCb onUpdate = nullptr;
};

class ZicApp {
public:
    SequenceBrain& brain;
    DriftKick& kick;
    DisplayView displayView;

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

    static constexpr int TOTAL_MENU_ITEMS = 14;
    MenuItem menuItems[TOTAL_MENU_ITEMS];

    ZicApp(SequenceBrain& b, DriftKick& k)
        : brain(b)
        , kick(k)
    {
        auto cbRegen = [](SequenceBrain& sb) { sb.regenerateKick(); };

        menuItems[0]  = { "PLAY / STOP", nullptr, nullptr, 0.0f, 1.0f, 1.0f, "", true };
        menuItems[1]  = { "BPM", nullptr, &brain.bpm, 60.0f, 240.0f, 1.0f, " BPM", true };
        menuItems[2]  = { "Gen Velocity", nullptr, &brain.genP1, 0.0f, 1.0f, 0.05f, "%", false, cbRegen };
        menuItems[3]  = { "Gen Ghosts", nullptr, &brain.genP2, 0.0f, 1.0f, 0.05f, "%", false, cbRegen };
        menuItems[4]  = { "Gen Rumble", nullptr, &brain.genP3, 0.0f, 1.0f, 0.05f, "%", false, cbRegen };
        menuItems[5]  = { "Sub Freq", &kick.baseFreq, nullptr, 30.0f, 100.0f, 1.0f, " Hz", true };
        menuItems[6]  = { "Click Amt", &kick.clickAmt, nullptr, 0.0f, 100.0f, 1.0f, "%", true };
        menuItems[7]  = { "Duration", &kick.duration, nullptr, 50.0f, 1500.0f, 10.0f, " ms", true };
        menuItems[8]  = { "VCO Morph", &kick.vcoMorph, nullptr, 0.0f, 100.0f, 1.0f, "%", true };
        menuItems[9]  = { "FM Depth", &kick.fmDepth, nullptr, 0.0f, 100.0f, 1.0f, "%", true };
        menuItems[10] = { "Drive", &kick.drive, nullptr, 0.0f, 100.0f, 1.0f, "%", true };
        menuItems[11] = { "Rumble", &kick.rumbleAmt, nullptr, 0.0f, 100.0f, 1.0f, "%", true };
        menuItems[12] = { "Rum Gap", &kick.rumbleGap, nullptr, 10.0f, 400.0f, 5.0f, " ms", true };
        menuItems[13] = { "Master Vol", nullptr, &masterVolume, 0.0f, 1.0f, 0.05f, "%", false };
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

    void getPotFormattedValue(PotIndex pot, char* buf, size_t size)
    {
        switch (pot) {
            case POT_SUB_FREQ: snprintf(buf, size, "%.0f Hz", kick.baseFreq.value); break;
            case POT_CLICK_AMT: snprintf(buf, size, "%.0f %%", kick.clickAmt.value); break;
            case POT_DURATION: snprintf(buf, size, "%.0f ms", kick.duration.value); break;
            case POT_VCO_MORPH: snprintf(buf, size, "%.0f %%", kick.vcoMorph.value); break;
            case POT_FM_DEPTH: snprintf(buf, size, "%.0f %%", kick.fmDepth.value); break;
            case POT_DRIVE: snprintf(buf, size, "%.0f %%", kick.drive.value); break;
            case POT_RUMBLE_AMT: snprintf(buf, size, "%.0f %%", kick.rumbleAmt.value); break;
            case POT_RUMBLE_GAP: snprintf(buf, size, "%.0f ms", kick.rumbleGap.value); break;
            case POT_BPM: snprintf(buf, size, "%.0f BPM", brain.bpm); break;
            case POT_MASTER_VOL: snprintf(buf, size, "%d %%", (int)std::round(masterVolume * 100.0f)); break;
            default: buf[0] = '\0'; break;
        }
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
            if (currentMenuItem < 0) currentMenuItem = TOTAL_MENU_ITEMS - 1;
            if (currentMenuItem >= TOTAL_MENU_ITEMS) currentMenuItem = 0;
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

    void getFormattedMenuItemValue(const MenuItem& item, int index, char* buf, size_t size)
    {
        if (index == 0) {
            snprintf(buf, size, "%s", brain.isPlaying ? "RUNNING" : "STOPPED");
            return;
        }
        float val = 0.0f;
        if (item.param != nullptr) {
            val = item.param->value;
        } else if (item.varPtr != nullptr) {
            val = *item.varPtr;
        }

        if (item.isInteger) {
            snprintf(buf, size, "%d%s", (int)std::round(val), item.unit);
        } else if (item.maxVal <= 1.0f && item.minVal >= 0.0f) {
            snprintf(buf, size, "%d%%", (int)std::round(val * 100.0f));
        } else {
            snprintf(buf, size, "%.1f%s", val, item.unit);
        }
    }

    void renderDisplay()
    {
        char potValBuf[32];
        getPotFormattedValue((PotIndex)lastMovedPotIndex, potValBuf, sizeof(potValBuf));

        char itemValBuf[32];
        getFormattedMenuItemValue(menuItems[currentMenuItem], currentMenuItem, itemValBuf, sizeof(itemValBuf));

        displayView.render(brain, kick, currentMenuItem, isEditing, TOTAL_MENU_ITEMS,
                           potOverlayTimer, lastMovedPotIndex, potValues[lastMovedPotIndex],
                           potValBuf, itemValBuf);
    }
};
