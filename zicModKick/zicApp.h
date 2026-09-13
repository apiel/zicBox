#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "audio/Eq.h"
#include "audio/engines/KickWave.h"
#include "audio/engines/PotKick.h"
#include "displayView.h"
#include "sequenceBrain.h"

enum PotIndex {
    POT_FM_DEPTH = 0,    // A10
    POT_DRIVE,           // A1
    POT_WAVE_SHAPE,      // A6
    POT_HARMONIC2,       // A5
    POT_HARMONIC3,       // A4
    POT_SKEW,            // A11
    POT_FOLD,            // A8
    POT_PHASE_OFFSET,    // A0
    POT_RESONATOR,       // A3
    POT_CRUSH,           // A2
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
    PotKick& potKick;
    KickWave& kickWave;
    DisplayView displayView;

    EQ masterEq;
    float eqLowVal = 0.0f;
    float eqMidVal = 0.0f;

    float engineIdxVal = 0.0f; // 0.0 = PotKick, 1.0 = KickWave
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
        0.35f, // FM Depth (35% default)
        0.35f, // Drive (35% default)
        0.00f, // Wave Shape (0% default)
        0.50f, // Harmonic 2 (0% default, centered -100 to 100)
        0.50f, // Harmonic 3 (0% default, centered -100 to 100)
        0.50f, // Wave Skew (50% default)
        0.00f, // Wave Fold (0% default)
        0.00f, // Phase Shift (0% default)
        0.00f, // Resonator (0% default)
        0.00f  // Crush (0% default)
    };

    static constexpr int MAX_MENU_ITEMS = 32;
    MenuItem menuItems[MAX_MENU_ITEMS];
    int totalMenuItems = 0;

    IEngine& getActiveEngine()
    {
        if (currentEngineIdx == 1) return kickWave;
        return potKick;
    }

    bool isBodyMuted()
    {
        return isPersistentBodyMuted || isTemporaryBodyMuted;
    }

    float processMasterEq(float sample)
    {
        if (eqLowVal != masterEq.gainDb[0] || eqMidVal != masterEq.gainDb[1]) {
            masterEq.gainDb[0] = eqLowVal;
            masterEq.gainDb[1] = eqMidVal;
            masterEq.recompute(44100.0f);
        }
        if (std::abs(eqLowVal) > 0.01f || std::abs(eqMidVal) > 0.01f) {
            return masterEq.process(sample);
        }
        return sample;
    }

    void updateMenuItems()
    {
        auto cbPattern = [](SequenceBrain& sb) { sb.loadPattern((int)std::round(sb.patternIdx)); };
        int idx = 0;

        menuItems[idx++] = { "Engine", nullptr, &engineIdxVal, 0.0f, 1.0f, 1.0f, "", true };
        menuItems[idx++] = { "Pattern", nullptr, &brain.patternIdx, 0.0f, (float)(SequenceBrain::NUM_PATTERNS - 1), 1.0f, "", true, cbPattern };
        menuItems[idx++] = { "Gen Pattern", nullptr, nullptr, 0.0f, 1.0f, 1.0f, "", true };
        menuItems[idx++] = { "EQ Low", nullptr, &eqLowVal, -12.0f, 12.0f, 0.5f, " dB", false };
        menuItems[idx++] = { "EQ Mid", nullptr, &eqMidVal, -12.0f, 12.0f, 0.5f, " dB", false };

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

        
        menuItems[idx++] = { "Rpt Rate", nullptr, nullptr, 1.0f, 8.0f, 1.0f, "x", true };
        menuItems[idx++] = { "Transpose", nullptr, &transposeSemitones, -24.0f, 24.0f, 1.0f, " st", true };
        menuItems[idx++] = { "PLAY / STOP", nullptr, nullptr, 0.0f, 1.0f, 1.0f, "", true };
        menuItems[idx++] = { "BPM", nullptr, &brain.bpm, 60.0f, 240.0f, 1.0f, " BPM", true };
        menuItems[idx++] = { "Master Vol", nullptr, &masterVolume, 0.0f, 1.0f, 0.05f, "%", false };

        totalMenuItems = idx;
        if (currentMenuItem >= totalMenuItems) {
            currentMenuItem = totalMenuItems - 1;
        }
    }

    ZicApp(SequenceBrain& b, PotKick& pk, KickWave& kw)
        : brain(b)
        , potKick(pk)
        , kickWave(kw)
    {
        masterEq.crossoverLow = 150.0f;
        masterEq.crossoverHigh = 3000.0f;
        masterEq.gainDb[0] = 0.0f;
        masterEq.gainDb[1] = 0.0f;
        masterEq.gainDb[2] = 0.0f;
        masterEq.recompute(44100.0f);

        updateMenuItems();
    }

    Param* getPotParamRef(PotIndex pot)
    {
        if (currentEngineIdx == 1) { // KickWave
            switch (pot) {
                case POT_FM_DEPTH: return &kickWave.fmDepth;
                case POT_DRIVE: return &kickWave.drive;
                case POT_WAVE_SHAPE: return &kickWave.waveShape;
                case POT_HARMONIC2: return &kickWave.harmonic2;
                case POT_HARMONIC3: return &kickWave.harmonic3;
                case POT_SKEW: return &kickWave.skew;
                case POT_FOLD: return &kickWave.fold;
                case POT_PHASE_OFFSET: return &kickWave.phaseOffset;
                case POT_RESONATOR: return &kickWave.resonator;
                case POT_CRUSH: return &kickWave.crush;
                default: return nullptr;
            }
        }
        // PotKick (currentEngineIdx == 0)
        switch (pot) {
            case POT_FM_DEPTH: return &potKick.duration;        // A10
            case POT_DRIVE: return &potKick.drive;              // A1
            case POT_WAVE_SHAPE: return &potKick.vcoMorph;      // A6
            case POT_HARMONIC2: return &potKick.fmDepth;        // A5
            case POT_HARMONIC3: return &potKick.fmSnap;         // A4
            case POT_SKEW: return &potKick.sweepDepth;          // A11
            case POT_FOLD: return &potKick.sweepShp;            // A8
            case POT_PHASE_OFFSET: return &potKick.wavefold;    // A0
            case POT_RESONATOR: return &potKick.crush;          // A3
            case POT_CRUSH: return &potKick.resonator;          // A2
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
            snprintf(buf, size, "%d %s", (int)std::round(p->value), p->unit ? p->unit : "");
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
            MenuItem& item = menuItems[currentMenuItem];
            if (item.varPtr == &engineIdxVal) { // Engine selection
                engineIdxVal = std::clamp(engineIdxVal + (dir > 0 ? 1.0f : -1.0f), 0.0f, 1.0f);
                currentEngineIdx = (int)std::round(engineIdxVal);
                updateMenuItems();
                potOverlayTimer = 0;
                for (int p = 0; p < NUM_POTS; ++p) {
                    applyPotValueSilent((PotIndex)p, potValues[p]);
                }
                return;
            }
            if (strcmp(item.name, "Rpt Rate") == 0) { // Rpt Rate
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
            if (currentMenuItem < 0) currentMenuItem = totalMenuItems - 1;
            if (currentMenuItem >= totalMenuItems) currentMenuItem = 0;
        }
    }

    void handleEncoderClick(const SequenceBrain::MidiTxFunc& txFunc = nullptr)
    {
        potOverlayTimer = 0;
        if (strcmp(menuItems[currentMenuItem].name, "PLAY / STOP") == 0) {
            brain.togglePlayStop(txFunc);
        } else if (strcmp(menuItems[currentMenuItem].name, "Gen Pattern") == 0) {
            brain.generateStyle16Pattern();
            isEditing = false;
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
                potKick.transposeSemitones = transposeSemitones;
                kickWave.transposeSemitones = transposeSemitones;
            } else if (brain.isPlaying) {
                brain.isNoteRepeatActive = true;
            } else {
                if (currentEngineIdx == 1) kickWave.noteOn(60, 1.0f);
                else potKick.noteOn(60, 1.0f);
            }
        } else {
            brain.isNoteRepeatActive = false;
            if (isTransposeActive) {
                isTransposeActive = false;
                potKick.transposeSemitones = 0.0f;
                kickWave.transposeSemitones = 0.0f;
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
                    potKick.isBodyMuted = isBodyMuted();
                    kickWave.isBodyMuted = isBodyMuted();
                } else {
                    if (currentEngineIdx == 1) kickWave.noteOn(60, 1.0f);
                    else potKick.noteOn(60, 1.0f);
                }
            }
        } else {
            isTemporaryBodyMuted = false;
            potKick.isBodyMuted = isBodyMuted();
            kickWave.isBodyMuted = isBodyMuted();
        }
    }

    void getFormattedMenuItemValue(const MenuItem& item, int index, char* buf, size_t size)
    {
        (void)index;
        if (item.varPtr == &engineIdxVal) { // Engine selection
            strncpy(buf, currentEngineIdx == 1 ? "KickWave" : "PotKick", size);
            return;
        }
        if (strcmp(item.name, "Gen Pattern") == 0) {
            strncpy(buf, "[GENERATE]", size);
            return;
        }
        if (strcmp(item.name, "PLAY / STOP") == 0) {
            strncpy(buf, brain.isPlaying ? "RUNNING" : "STOPPED", size);
            return;
        }
        if (item.varPtr == &transposeSemitones) {
            int semitones = (int)std::round(transposeSemitones);
            snprintf(buf, size, semitones > 0 ? "+%d st" : "%d st", semitones);
            return;
        }
        if (strcmp(item.name, "Rpt Rate") == 0) {
            if (brain.repeatDiv == 1) strncpy(buf, "1 step", size);
            else snprintf(buf, size, "%d steps", brain.repeatDiv);
            return;
        }
        if (item.varPtr == &brain.patternIdx) {
            int pIdx = (int)std::round(brain.patternIdx);
            snprintf(buf, size, "%s", brain.getPatternName(pIdx));
            return;
        }
        float val = item.param != nullptr ? item.param->value : (item.varPtr != nullptr ? *item.varPtr : 0.0f);

        if (item.isInteger) {
            snprintf(buf, size, "%d%s", (int)std::round(val), item.unit ? item.unit : "");
        } else if (item.maxVal <= 1.0f && item.minVal >= 0.0f) {
            snprintf(buf, size, "%d%%", (int)std::round(val * 100.0f));
        } else {
            int mainPart = (int)val;
            int decPart = (int)std::abs(std::round((val - mainPart) * 10.0f));
            snprintf(buf, size, "%d.%d%s", mainPart, decPart, item.unit ? item.unit : "");
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
