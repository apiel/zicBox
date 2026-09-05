#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "audio/engines/PotKick.h"
#include "displayView.h"
#include "sequenceBrain.h"

enum PotIndex {
    POT_DURATION = 0, // A10
    POT_VCO_MORPH,    // A6
    POT_FM_DEPTH,     // A5
    POT_FM_SNAP,      // A4
    POT_SWEEP_DEPTH,  // A11
    POT_SWEEP_SHP,    // A8
    POT_DRIVE,        // A1
    POT_WAVEFOLD,     // A0
    POT_CRUSH,        // A3
    POT_RESONATOR,    // A2
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
    PotKick& kick;
    DisplayView displayView;

    float masterVolume = 0.8f;
    int currentMenuItem = 0;
    bool isEditing = false;

    // Pot overlay state
    int lastMovedPotIndex = -1;
    int32_t potOverlayTimer = 0; // ms or frames

    // Virtual Potentiometer normalized values [0.0, 1.0]
    float potValues[NUM_POTS] = {
        0.20f, // Duration (350 ms default)
        0.00f, // VCO Morph (0% default)
        0.25f, // FM Depth (25% default)
        0.16f, // FM Snap (25 ms default)
        0.50f, // Sweep Depth (50% default)
        0.50f, // Sweep Shape (50% default)
        0.35f, // Drive (35% default)
        0.00f, // Wavefold (0% default)
        0.00f, // Crush (0% default)
        0.00f  // Resonator (0% default)
    };

    static constexpr int TOTAL_MENU_ITEMS = 14;
    MenuItem menuItems[TOTAL_MENU_ITEMS];

    ZicApp(SequenceBrain& b, PotKick& k)
        : brain(b)
        , kick(k)
    {
        auto cbRegen = [](SequenceBrain& sb) { sb.regenerateKick(); };

        menuItems[0]  = { "PLAY / STOP", nullptr, nullptr, 0.0f, 1.0f, 1.0f, "", true };
        menuItems[1]  = { "BPM", nullptr, &brain.bpm, 60.0f, 240.0f, 1.0f, " BPM", true };
        menuItems[2]  = { "Master Vol", nullptr, &masterVolume, 0.0f, 1.0f, 0.05f, "%", false };
        menuItems[3]  = { "Sub Freq", &kick.baseFreq, nullptr, 30.0f, 100.0f, 1.0f, " Hz", true };
        menuItems[4]  = { "FM Ratio", &kick.fmRatio, nullptr, 0.5f, 8.0f, 0.25f, "x", false };
        menuItems[5]  = { "Click Amt", &kick.kickClickAmt, nullptr, 0.0f, 100.0f, 1.0f, "%", true };
        menuItems[6]  = { "Click Dec", &kick.kickClickDecay, nullptr, 1.0f, 100.0f, 1.0f, " ms", true };
        menuItems[7]  = { "Bass Boost", &kick.bassBoost, nullptr, 0.0f, 100.0f, 1.0f, "%", true };
        menuItems[8]  = { "EQ Low", &kick.eqLow, nullptr, -12.0f, 12.0f, 0.5f, " dB", false };
        menuItems[9]  = { "EQ Mid", &kick.eqMid, nullptr, -12.0f, 12.0f, 0.5f, " dB", false };
        menuItems[10] = { "EQ High", &kick.eqHigh, nullptr, -12.0f, 12.0f, 0.5f, " dB", false };
        menuItems[11] = { "Gen Velocity", nullptr, &brain.genP1, 0.0f, 1.0f, 0.05f, "%", false, cbRegen };
        menuItems[12] = { "Gen Ghosts", nullptr, &brain.genP2, 0.0f, 1.0f, 0.05f, "%", false, cbRegen };
        menuItems[13] = { "Gen Rumble", nullptr, &brain.genP3, 0.0f, 1.0f, 0.05f, "%", false, cbRegen };
    }

    const char* getPotName(PotIndex pot)
    {
        switch (pot) {
            case POT_DURATION: return "Duration";
            case POT_VCO_MORPH: return "VCO Morph";
            case POT_FM_DEPTH: return "FM Depth";
            case POT_FM_SNAP: return "FM Snap";
            case POT_SWEEP_DEPTH: return "Sweep Depth";
            case POT_SWEEP_SHP: return "Sweep Shape";
            case POT_DRIVE: return "Drive";
            case POT_WAVEFOLD: return "Wavefold";
            case POT_CRUSH: return "Crush";
            case POT_RESONATOR: return "Resonator";
            default: return "";
        }
    }

    void getPotFormattedValue(PotIndex pot, char* buf, size_t size)
    {
        switch (pot) {
            case POT_DURATION: snprintf(buf, size, "%d ms", (int)std::round(kick.duration.value)); break;
            case POT_VCO_MORPH: snprintf(buf, size, "%d %%", (int)std::round(kick.vcoMorph.value)); break;
            case POT_FM_DEPTH: snprintf(buf, size, "%d %%", (int)std::round(kick.fmDepth.value)); break;
            case POT_FM_SNAP: snprintf(buf, size, "%d ms", (int)std::round(kick.fmSnap.value)); break;
            case POT_SWEEP_DEPTH: snprintf(buf, size, "%d %%", (int)std::round(kick.sweepDepth.value)); break;
            case POT_SWEEP_SHP: snprintf(buf, size, "%d %%", (int)std::round(kick.sweepShp.value)); break;
            case POT_DRIVE: snprintf(buf, size, "%d %%", (int)std::round(kick.drive.value)); break;
            case POT_WAVEFOLD: snprintf(buf, size, "%d %%", (int)std::round(kick.wavefold.value)); break;
            case POT_CRUSH: snprintf(buf, size, "%d %%", (int)std::round(kick.crush.value)); break;
            case POT_RESONATOR: snprintf(buf, size, "%d %%", (int)std::round(kick.resonator.value)); break;
            default: buf[0] = '\0'; break;
        }
    }

    void applyPotValue(PotIndex pot, float normVal)
    {
        normVal = std::clamp(normVal, 0.0f, 1.0f);
        potValues[pot] = normVal;

        switch (pot) {
            case POT_DURATION:
                kick.duration.set(kick.duration.min + normVal * (kick.duration.max - kick.duration.min));
                break;
            case POT_VCO_MORPH:
                kick.vcoMorph.set(kick.vcoMorph.min + normVal * (kick.vcoMorph.max - kick.vcoMorph.min));
                break;
            case POT_FM_DEPTH:
                kick.fmDepth.set(kick.fmDepth.min + normVal * (kick.fmDepth.max - kick.fmDepth.min));
                break;
            case POT_FM_SNAP:
                kick.fmSnap.set(kick.fmSnap.min + normVal * (kick.fmSnap.max - kick.fmSnap.min));
                break;
            case POT_SWEEP_DEPTH:
                kick.sweepDepth.set(kick.sweepDepth.min + normVal * (kick.sweepDepth.max - kick.sweepDepth.min));
                break;
            case POT_SWEEP_SHP:
                kick.sweepShp.set(kick.sweepShp.min + normVal * (kick.sweepShp.max - kick.sweepShp.min));
                break;
            case POT_DRIVE:
                kick.drive.set(kick.drive.min + normVal * (kick.drive.max - kick.drive.min));
                break;
            case POT_WAVEFOLD:
                kick.wavefold.set(kick.wavefold.min + normVal * (kick.wavefold.max - kick.wavefold.min));
                break;
            case POT_CRUSH:
                kick.crush.set(kick.crush.min + normVal * (kick.crush.max - kick.crush.min));
                break;
            case POT_RESONATOR:
                kick.resonator.set(kick.resonator.min + normVal * (kick.resonator.max - kick.resonator.min));
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
            int mainPart = (int)val;
            int decPart = (int)std::abs(std::round((val - mainPart) * 10.0f));
            snprintf(buf, size, "%d.%d%s", mainPart, decPart, item.unit);
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
