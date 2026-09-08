#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "audio/engines/PotKick.h"
#include "audio/engines/PotWavKick.h"
#include "displayView.h"
#include "sequenceBrain.h"

enum PotIndex {
    POT_DURATION = 0, // A10
    POT_VCO_MORPH,    // A6
    POT_PAR2,         // A5 - FM Depth (PotKick) / Symmetry (PotWavKick)
    POT_PAR3,         // A4 - FM Snap (PotKick) / Trans Bite (PotWavKick)
    POT_SWEEP_DEPTH,  // A11
    POT_SWEEP_SHP,    // A8
    POT_DRIVE,        // A1
    POT_PAR7,         // A0 - Bass Boost (PotKick) / Formant Fold (PotWavKick)
    POT_PAR8,         // A3 - Click Amt (PotKick) / Sub Punch (PotWavKick)
    POT_PAR9,         // A2 - Click Dec (PotKick) / Tone Filter (PotWavKick)
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
    PotWavKick& wavKick;
    DisplayView displayView;

    float engineIdxVal = 0.0f; // 0.0 = PotKick, 1.0 = PotWavKick
    int currentEngineIdx = 0;

    float masterVolume = 0.5f;
    int currentMenuItem = 0;
    bool isEditing = false;

    // Button states
    bool isShiftPressed = false;
    bool isTemporaryBodyMuted = false;
    bool isPersistentBodyMuted = false;

    // Transpose state
    float transposeSemitones = 0.0f;
    bool isTransposeActive = false;

    // Pot overlay state
    int lastMovedPotIndex = -1;
    int32_t potOverlayTimer = 0; // ms or frames

    // Virtual Potentiometer normalized values [0.0, 1.0]
    float potValues[NUM_POTS] = {
        0.20f, // Duration (350 ms default)
        0.20f, // VCO Morph (20% default)
        0.50f, // FM Depth / Symmetry (50% default)
        0.25f, // FM Snap / Trans Bite (25% default)
        0.50f, // Sweep Depth (50% default)
        0.50f, // Sweep Shape (50% default)
        0.30f, // Drive (30% default)
        0.00f, // Bass Boost / Formant (0% default)
        0.30f, // Click Amt / Sub Punch (30% default)
        0.75f  // Click Dec / Tone (75% default)
    };

    static constexpr int MAX_MENU_ITEMS = 32;
    MenuItem menuItems[MAX_MENU_ITEMS];
    int totalMenuItems = 0;

    IEngine& getActiveEngine()
    {
        if (currentEngineIdx == 1) return wavKick;
        return kick;
    }

    bool isBodyMuted()
    {
        return isPersistentBodyMuted || isTemporaryBodyMuted;
    }

    void updateMenuItems()
    {
        auto cbRegen = [](SequenceBrain& sb) { sb.regenerateKick(); };
        int idx = 0;

        menuItems[idx++] = { "Engine", nullptr, &engineIdxVal, 0.0f, 1.0f, 1.0f, "", true };
        menuItems[idx++] = { "BPM", nullptr, &brain.bpm, 60.0f, 240.0f, 1.0f, " BPM", true };
        menuItems[idx++] = { "Master Vol", nullptr, &masterVolume, 0.0f, 1.0f, 0.05f, "%", false };

        IEngine& eng = getActiveEngine();
        Param* pArray = eng.getParams();
        size_t pCount = eng.getParamCount();

        for (size_t i = 0; i < pCount; i++) {
            Param& p = pArray[i];
            if (isPotParam(&p)) {
                continue; // Exclude parameters that are available through pots!
            }
            menuItems[idx++] = {
                p.label,
                &p,
                nullptr,
                p.min,
                p.max,
                p.step,
                p.unit,
                (p.step >= 1.0f)
            };
        }

        menuItems[idx++] = { "Gen Velocity", nullptr, &brain.genP1, 0.0f, 1.0f, 0.05f, "%", false, cbRegen };
        menuItems[idx++] = { "Gen Ghosts", nullptr, &brain.genP2, 0.0f, 1.0f, 0.05f, "%", false, cbRegen };
        menuItems[idx++] = { "Gen Rumble", nullptr, &brain.genP3, 0.0f, 1.0f, 0.05f, "%", false, cbRegen };
        menuItems[idx++] = { "Rpt Rate", nullptr, nullptr, 1.0f, 8.0f, 1.0f, "x", true };
        menuItems[idx++] = { "Transpose", nullptr, &transposeSemitones, -24.0f, 24.0f, 1.0f, " st", true };
        menuItems[idx++] = { "PLAY / STOP", nullptr, nullptr, 0.0f, 1.0f, 1.0f, "", true };

        totalMenuItems = idx;
        if (currentMenuItem >= totalMenuItems) {
            currentMenuItem = totalMenuItems - 1;
        }
    }

    ZicApp(SequenceBrain& b, PotKick& k, PotWavKick& wk)
        : brain(b)
        , kick(k)
        , wavKick(wk)
    {
        updateMenuItems();
    }

    Param* getPotParamRef(PotIndex pot)
    {
        if (currentEngineIdx == 1) { // PotWavKick
            switch (pot) {
                case POT_DURATION: return &wavKick.duration;
                case POT_VCO_MORPH: return &wavKick.vcoMorph;
                case POT_PAR2: return &wavKick.phaseSym;
                case POT_PAR3: return &wavKick.transientBite;
                case POT_SWEEP_DEPTH: return &wavKick.sweepDepth;
                case POT_SWEEP_SHP: return &wavKick.sweepShp;
                case POT_DRIVE: return &wavKick.drive;
                case POT_PAR7: return &wavKick.formantFold;
                case POT_PAR8: return &wavKick.subPunch;
                case POT_PAR9: return &wavKick.toneFilter;
                default: return nullptr;
            }
        }
        // PotKick
        switch (pot) {
            case POT_DURATION: return &kick.duration;
            case POT_VCO_MORPH: return &kick.vcoMorph;
            case POT_PAR2: return &kick.fmDepth;
            case POT_PAR3: return &kick.fmSnap;
            case POT_SWEEP_DEPTH: return &kick.sweepDepth;
            case POT_SWEEP_SHP: return &kick.sweepShp;
            case POT_DRIVE: return &kick.drive;
            case POT_PAR7: return &kick.wavefold;
            case POT_PAR8: return &kick.crush;
            case POT_PAR9: return &kick.resonator;
            default: return nullptr;
        }
    }

    bool isPotParam(Param* p)
    {
        for (int i = 0; i < NUM_POTS; ++i) {
            if (getPotParamRef((PotIndex)i) == p) return true;
        }
        return false;
    }

    const char* getPotName(PotIndex pot)
    {
        Param* p = getPotParamRef(pot);
        return p ? p->label : "";
    }

    void getPotFormattedValue(PotIndex pot, char* buf, size_t size)
    {
        Param* p = getPotParamRef(pot);
        if (p) {
            snprintf(buf, size, "%d %s", (int)std::round(p->value), p->unit);
        } else {
            buf[0] = '\0';
        }
    }

    void applyPotValueInternal(PotIndex pot, float normVal)
    {
        normVal = std::clamp(normVal, 0.0f, 1.0f);
        potValues[pot] = normVal;
        Param* p = getPotParamRef(pot);
        if (p) {
            p->set(p->min + normVal * (p->max - p->min));
        }
    }

    void applyPotValueSilent(PotIndex pot, float normVal)
    {
        applyPotValueInternal(pot, normVal);
    }

    void applyPotValue(PotIndex pot, float normVal)
    {
        applyPotValueInternal(pot, normVal);
        lastMovedPotIndex = (int)pot;
        potOverlayTimer = 1500; // 1.5 seconds overlay
    }

    void handleEncoderTurn(int dir)
    {
        potOverlayTimer = 0; // Clear takeover overlay immediately
        if (isEditing) {
            if (currentMenuItem == 0) { // Engine selection
                engineIdxVal = std::clamp(engineIdxVal + (dir > 0 ? 1.0f : -1.0f), 0.0f, 1.0f);
                currentEngineIdx = (int)std::round(engineIdxVal);
                updateMenuItems();
                potOverlayTimer = 0; // Ensure overlay remains clear when changing engine
                for (int p = 0; p < NUM_POTS; ++p) {
                    applyPotValueSilent((PotIndex)p, potValues[p]);
                }
                return;
            }
            if (currentMenuItem == totalMenuItems - 3) { // Rpt Rate
                if (dir > 0) {
                    if (brain.repeatDiv == 1) brain.repeatDiv = 2;
                    else if (brain.repeatDiv == 2) brain.repeatDiv = 4;
                    else if (brain.repeatDiv == 4) brain.repeatDiv = 8;
                } else if (dir < 0) {
                    if (brain.repeatDiv == 8) brain.repeatDiv = 4;
                    else if (brain.repeatDiv == 4) brain.repeatDiv = 2;
                    else if (brain.repeatDiv == 2) brain.repeatDiv = 1;
                }
                return;
            }
            MenuItem& item = menuItems[currentMenuItem];
            float step = item.stepVal > 0.0f ? item.stepVal : 1.0f;
            if (item.param != nullptr) {
                item.param->set(item.param->value + (dir * step));
            } else if (item.varPtr != nullptr) {
                *item.varPtr = std::clamp(*item.varPtr + (dir * step), item.minVal, item.maxVal);
                if (currentMenuItem == 0) {
                    currentEngineIdx = (int)std::round(engineIdxVal);
                    updateMenuItems();
                    potOverlayTimer = 0;
                    for (int p = 0; p < NUM_POTS; ++p) {
                        applyPotValueSilent((PotIndex)p, potValues[p]);
                    }
                }
            }
            if (item.onUpdate != nullptr) {
                item.onUpdate(brain);
            }
        } else {
            currentMenuItem += (dir > 0 ? 1 : -1);
            if (currentMenuItem < 0) currentMenuItem = totalMenuItems - 1;
            if (currentMenuItem >= totalMenuItems) currentMenuItem = 0;
        }
    }

    void handleEncoderClick(const SequenceBrain::MidiTxFunc& txFunc = nullptr)
    {
        potOverlayTimer = 0;
        if (currentMenuItem == totalMenuItems - 1) { // PLAY / STOP (last item)
            brain.togglePlayStop(txFunc);
        } else {
            isEditing = !isEditing;
        }
    }

    void handleButton1(bool pressed)
    {
        isShiftPressed = pressed;
    }

    void handleButton2(bool pressed, const SequenceBrain::MidiTxFunc& txFunc = nullptr)
    {
        if (pressed) {
            if (isShiftPressed) {
                isTransposeActive = true;
                kick.transposeSemitones = transposeSemitones;
                wavKick.transposeSemitones = transposeSemitones;
            } else if (brain.isPlaying) {
                brain.isNoteRepeatActive = true;
            } else {
                if (currentEngineIdx == 1) wavKick.noteOn(60, 1.0f);
                else kick.noteOn(60, 1.0f);
            }
        } else {
            brain.isNoteRepeatActive = false;
            if (isTransposeActive) {
                isTransposeActive = false;
                kick.transposeSemitones = 0.0f;
                wavKick.transposeSemitones = 0.0f;
            }
        }
    }

    void handleButton3(bool pressed, const SequenceBrain::MidiTxFunc& txFunc = nullptr)
    {
        if (pressed) {
            if (isShiftPressed) {
                brain.togglePlayStop(txFunc);
            } else {
                if (brain.isPlaying) {
                    isTemporaryBodyMuted = true;
                    kick.isBodyMuted = isBodyMuted();
                    wavKick.isBodyMuted = isBodyMuted();
                } else {
                    if (currentEngineIdx == 1) wavKick.noteOn(60, 1.0f);
                    else kick.noteOn(60, 1.0f);
                }
            }
        } else {
            isTemporaryBodyMuted = false;
            kick.isBodyMuted = isBodyMuted();
            wavKick.isBodyMuted = isBodyMuted();
        }
    }

    void getFormattedMenuItemValue(const MenuItem& item, int index, char* buf, size_t size)
    {
        if (index == 0) { // Engine selection
            strncpy(buf, currentEngineIdx == 1 ? "PotWav" : "PotKick", size);
            return;
        }
        if (index == totalMenuItems - 1) { // PLAY / STOP
            strncpy(buf, brain.isPlaying ? "RUNNING" : "STOPPED", size);
            return;
        }
        if (index == totalMenuItems - 2) { // Transpose
            int semitones = (int)std::round(transposeSemitones);
            snprintf(buf, size, semitones > 0 ? "+%d st" : "%d st", semitones);
            return;
        }
        if (index == totalMenuItems - 3) { // Rpt Rate
            if (brain.repeatDiv == 1) strncpy(buf, "1 step", size);
            else snprintf(buf, size, "%d steps", brain.repeatDiv);
            return;
        }
        float val = item.param != nullptr ? item.param->value : (item.varPtr != nullptr ? *item.varPtr : 0.0f);

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

        displayView.render(brain, getActiveEngine(), isBodyMuted(), currentMenuItem, isEditing, totalMenuItems,
                           potOverlayTimer, lastMovedPotIndex, potValues[lastMovedPotIndex],
                           potValBuf, itemValBuf, menuItems[currentMenuItem].name, currentEngineIdx);
    }
};
