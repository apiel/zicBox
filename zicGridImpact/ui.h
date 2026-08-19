#pragma once

#include "draw/draw.h"
#include "draw/utils/Icon.h"
#include "ui/uiParams.h"
#include "zicGridImpact/gridState.h"
#include "zicGridImpact/studio.h"

#include "zicGridImpact/views/MasterView.h"
#include "zicGridImpact/views/SequencerView.h"
#include "zicGridImpact/views/KickView.h"
#include "zicGridImpact/views/Synth1View.h"
#include "zicGridImpact/views/Synth2View.h"
#include "zicGridImpact/views/ChaosView.h"

#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"

inline MasterView masterView;
inline SequencerView seqView;
inline KickView kickView;
inline Synth1View synth1View;
inline Synth2View synth2View;
inline ChaosView chaosView;

inline void updateActiveViewEncoders() {
    switch (gridState.activeView) {
        case VIEW_MASTER: masterView.updateEncoders(); break;
        case VIEW_SEQUENCER: seqView.updateEncoders(); break;
        case VIEW_KICK: kickView.updateEncoders(); break;
        case VIEW_SYNTH1: synth1View.updateEncoders(); break;
        case VIEW_SYNTH2: synth2View.updateEncoders(); break;
        case VIEW_CHAOS: chaosView.updateEncoders(); break;
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
        case VIEW_CHAOS: chaosView.handleEncoder(encoderIdx, delta); break;
    }
}

struct ScatterPadDef {
    const char* label;
    Color color;
    int mode; // 0..6
    float params[4]; // parameters (-1.0f means keep default)
};

// Category Unified Colors:
// Green:  Color { 0, 230, 130, 255 } (Gater / Stutter)
// Pink:   Color { 255, 60, 160, 255 } (Lo-Fi / Bitcrush)
// Gold:   Color { 230, 210, 0, 255 }  (Acid / Filter)
// Orange: Color { 255, 140, 0, 255 }  (Drive / Distortion)
// Teal:   Color { 0, 200, 255, 255 }  (Flanger / Comb)
// Purple: Color { 180, 90, 255, 255 } (Space / Delay)

// Row 1 Scatter Pads (Col 2..11, 0-indexed)
inline const ScatterPadDef scatterRow1[10] = {
    { "Gtr 4",  Color { 0, 230, 130, 255 }, 1, { 0.25f, 0.50f, 0.8f, 0.05f } }, // Green (Gater)
    { "Gtr 8",  Color { 0, 230, 130, 255 }, 1, { 0.50f, 0.50f, 0.8f, 0.05f } }, // Green (Gater)
    { "8-Bit",  Color { 255, 60, 160, 255 }, 4, { 0.40f, 0.10f, 0.10f, 0.70f } }, // Pink (Bitcrush)
    { "Crush",  Color { 255, 60, 160, 255 }, 4, { 0.10f, 0.60f, 0.20f, 0.50f } }, // Pink (Bitcrush)
    { "LP Fl",  Color { 230, 210, 0, 255 }, 6, { 0.20f, 0.50f, 0.0f, 0.0f } }, // Gold (Acid/Filter)
    { "HP Fl",  Color { 230, 210, 0, 255 }, 6, { 0.80f, 0.70f, 0.0f, 0.0f } }, // Gold (Acid/Filter)
    { "Drive",  Color { 255, 140, 0, 255 }, 5, { 0.60f, 0.20f, 0.0f, 0.3f } }, // Orange (Drive)
    { "Satur",  Color { 255, 140, 0, 255 }, 5, { 0.20f, 0.80f, 0.0f, 0.0f } }, // Orange (Drive)
    { "Flang",  Color { 0, 200, 255, 255 }, 0, { 0.80f, 4.00f, 0.4f, 0.7f } }, // Teal (Comb)
    { "Slap",   Color { 180, 90, 255, 255 }, 3, { 0.30f, 0.25f, 0.3f, 0.5f } }  // Purple (Delay)
};

// Row 2 Scatter Pads (Col 2..11, 0-indexed)
inline const ScatterPadDef scatterRow2[10] = {
    { "Gtr16",  Color { 0, 230, 130, 255 }, 1, { 1.00f, 0.50f, 0.9f, 0.05f } }, // Green (Gater)
    { "Gtr32",  Color { 0, 230, 130, 255 }, 1, { 2.00f, 0.50f, 1.0f, 0.01f } }, // Green (Gater)
    { "Squez",  Color { 255, 60, 160, 255 }, 2, { 0.20f, 0.50f, 0.0f, 0.0f } }, // Pink (Bitcrush)
    { "Decim",  Color { 255, 60, 160, 255 }, 2, { 0.75f, 0.0f, 0.0f, 0.0f } },  // Pink (Bitcrush)
    { "Acid",   Color { 230, 210, 0, 255 }, 6, { 0.60f, 0.85f, 0.2f, 0.0f } }, // Gold (Acid/Filter)
    { "Screm",  Color { 230, 210, 0, 255 }, 6, { 0.40f, 0.98f, 0.4f, 0.0f } }, // Gold (Acid/Filter)
    { "Dist",   Color { 255, 140, 0, 255 }, 2, { 0.40f, 0.00f, 0.5f, 0.6f } }, // Orange (Drive)
    { "Fold",   Color { 255, 140, 0, 255 }, 5, { 0.20f, 0.90f, 0.5f, 0.4f } }, // Orange (Drive)
    { "Comb",   Color { 0, 200, 255, 255 }, 0, { 0.95f, 8.00f, 0.8f, 0.9f } }, // Teal (Comb)
    { "Echo",   Color { 180, 90, 255, 255 }, 3, { 0.40f, 1.00f, 0.6f, 0.7f } }  // Purple (Delay)
};

// Row 3 Scatter Pads (Col 2..10, 0-indexed)
inline const ScatterPadDef scatterRow3[12] = {
    {}, {}, // Col 0 (Break), Col 1 (Drop)
    { "Chop",   Color { 0, 230, 130, 255 }, 1, { 4.00f, 0.50f, 1.0f, 0.80f } }, // Green (Gater)
    { "Roll",   Color { 0, 230, 130, 255 }, 1, { 4.00f, 0.50f, 1.0f, 0.05f } }, // Green (Gater)
    { "Crunch", Color { 255, 60, 160, 255 }, 4, { 0.35f, 0.10f, 0.10f, 0.70f } },// Pink (Bitcrush)
    { "Gltch",  Color { 255, 60, 160, 255 }, 4, { 0.90f, 0.40f, 0.5f, 0.4f } }, // Pink (Bitcrush)
    { "Ring",   Color { 230, 210, 0, 255 }, 6, { 0.35f, 0.50f, 0.0f, 0.7f } }, // Gold (Acid/Filter)
    { "Reso",   Color { 230, 210, 0, 255 }, 6, { 0.50f, 0.90f, 0.3f, 0.0f } }, // Gold (Acid/Filter)
    { "Over",   Color { 255, 140, 0, 255 }, 5, { 0.85f, 0.30f, 0.0f, 0.0f } }, // Orange (Drive)
    { "Clip",   Color { 255, 140, 0, 255 }, 5, { 0.30f, 0.00f, 0.9f, 0.0f } }, // Orange (Drive)
    { "LFO",    Color { 0, 200, 255, 255 }, 0, { 0.85f, 12.0f, 0.6f, 0.8f } }, // Teal (Comb)
    {} // Col 11 (Hold)
};

inline void processPerformancePadState() {
    gridState.isPressedA = gridState.pads[0][3].pressed;          // Break (Col 0, Row 3)
    gridState.isPressedS = gridState.pads[0][2].pressed;          // Repeat (Col 0, Row 2)
    gridState.isPressedRndBar = gridState.pads[0][1].pressed;     // RndBar (Col 0, Row 1)
    gridState.isPressedPlus2sm = gridState.pads[1][1].pressed;    // +2sm (Col 1, Row 1)
    gridState.isPressedMinus2sm = gridState.pads[1][2].pressed;   // -2sm (Col 1, Row 2)
    gridState.isPressedSubDrop = gridState.pads[1][3].pressed;    // Drop (Col 1, Row 3)
    gridState.isPressedZ = gridState.pads[4][3].pressed;          // Crunch (Col 4, Row 3)
    gridState.isPressedX = gridState.pads[5][3].pressed;          // Drive (Col 5, Row 3)
    gridState.isPressedC = gridState.pads[6][3].pressed;          // Dist (Col 6, Row 3)
    gridState.isPressedV = gridState.pads[7][3].pressed;          // Acid (Col 7, Row 3)

    // Latch processing (Pad 11, Row 3 -> Hold completely on right)
    if (gridState.pads[11][3].pressed) {
        if (gridState.isPressedA) gridState.isLatchedA = !gridState.isLatchedA;
        if (gridState.isPressedS) gridState.isLatchedS = !gridState.isLatchedS;
        if (gridState.isPressedRndBar) gridState.isLatchedRndBar = !gridState.isLatchedRndBar;
        if (gridState.isPressedPlus2sm) gridState.isLatchedPlus2sm = !gridState.isLatchedPlus2sm;
        if (gridState.isPressedMinus2sm) gridState.isLatchedMinus2sm = !gridState.isLatchedMinus2sm;
        if (gridState.isPressedSubDrop) gridState.isLatchedSubDrop = !gridState.isLatchedSubDrop;
        if (gridState.isPressedZ) gridState.isLatchedZ = !gridState.isLatchedZ;
        if (gridState.isPressedX) gridState.isLatchedX = !gridState.isLatchedX;
        if (gridState.isPressedC) gridState.isLatchedC = !gridState.isLatchedC;
        if (gridState.isPressedV) gridState.isLatchedV = !gridState.isLatchedV;

        for (int r = 1; r <= 3; ++r) {
            for (int c = 0; c < PAD_COLS; ++c) {
                if (gridState.pads[c][r].pressed && !gridState.isPressedScatter[c][r]) {
                    gridState.isLatchedScatter[c][r] = !gridState.isLatchedScatter[c][r];
                }
            }
        }
    }

    studio.kick.isBodyMuted = gridState.isLatchedA || gridState.isPressedA;
    studio.seq.isKickRepeatActive = gridState.isLatchedS || gridState.isPressedS;
    studio.seq.isKickRandomBarActive = gridState.isLatchedRndBar || gridState.isPressedRndBar;
    studio.kick.isSubDropActive = gridState.isLatchedSubDrop || gridState.isPressedSubDrop;

    int semitones = 0;
    if (gridState.isLatchedPlus2sm || gridState.isPressedPlus2sm) semitones += 2;
    if (gridState.isLatchedMinus2sm || gridState.isPressedMinus2sm) semitones -= 2;
    studio.kick.semitoneOffset = semitones;

    bool scatterActive[8] = { false };
    if (gridState.isLatchedZ || gridState.isPressedZ) scatterActive[4] = true;
    if (gridState.isLatchedX || gridState.isPressedX) scatterActive[5] = true;
    if (gridState.isLatchedC || gridState.isPressedC) scatterActive[2] = true;
    if (gridState.isLatchedV || gridState.isPressedV) scatterActive[6] = true;

    for (int r = 1; r <= 3; ++r) {
        for (int c = 0; c < PAD_COLS; ++c) {
            bool isScatterPad = false;
            const ScatterPadDef* def = nullptr;

            if (r == 1 && c >= 2) { isScatterPad = true; def = &scatterRow1[c - 2]; }
            else if (r == 2 && c >= 2) { isScatterPad = true; def = &scatterRow2[c - 2]; }
            else if (r == 3 && c >= 2 && c <= 10) { isScatterPad = true; def = &scatterRow3[c]; }

            if (isScatterPad && def && def->label) {
                bool pressed = gridState.pads[c][r].pressed;
                gridState.isPressedScatter[c][r] = pressed;

                bool isActive = gridState.isLatchedScatter[c][r] || pressed;
                if (isActive) {
                    scatterActive[def->mode] = true;
                    if (def->params[0] >= 0.0f) studio.scatter.params[def->mode][0] = def->params[0];
                    if (def->params[1] >= 0.0f) studio.scatter.params[def->mode][1] = def->params[1];
                    if (def->params[2] >= 0.0f) studio.scatter.params[def->mode][2] = def->params[2];
                    if (def->params[3] >= 0.0f) studio.scatter.params[def->mode][3] = def->params[3];
                }
            }
        }
    }

    for (int m = 0; m < 8; ++m) {
        studio.scatter.setModeActive(m, scatterActive[m]);
    }
}

inline std::vector<float> kickParamBackup;
inline std::vector<float> synth1ParamBackup;
inline std::vector<float> synth2ParamBackup;
inline std::vector<float> chaosParamBackup;
inline int lastRandomizedEngine = -1; // 0=Kick, 1=Synth1, 2=Synth2, 3=Chaos
inline bool revertUsedWithSynth = false;

template <typename EngineType>
inline void randomizeEngine(EngineType& eng, std::vector<float>& backup, int engineId) {
    Param* params = eng.getParams();
    size_t count = eng.getParamCount();
    backup.clear();
    backup.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        backup.push_back(params[i].value);
    }
    lastRandomizedEngine = engineId;

    for (size_t i = 0; i < count; ++i) {
        float minV = params[i].min;
        float maxV = params[i].max;
        float stepV = params[i].step;
        if (maxV <= minV) continue;
        float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        float rndVal = minV + r * (maxV - minV);
        if (stepV >= 1.0f && std::floor(stepV) == stepV) {
            rndVal = minV + std::round((rndVal - minV) / stepV) * stepV;
        }
        params[i].set(rndVal);
    }
}

template <typename EngineType>
inline bool revertEngine(EngineType& eng, const std::vector<float>& backup) {
    Param* params = eng.getParams();
    size_t count = eng.getParamCount();
    if (backup.empty() || backup.size() != count) return false;
    for (size_t i = 0; i < count; ++i) {
        params[i].set(backup[i]);
    }
    return true;
}

inline void handlePadPress(int col, int row, bool pressed) {
    gridState.pads[col][row].pressed = pressed;

    if (pressed) {
        // If shutdown confirmation modal is active, any pad press interacts with confirmation
        if (gridState.isShutdownConfirmModal) {
            if (row == 0 && col == 11) {
                // Shutdown pad (P3) pressed again -> confirm shutdown!
                gridState.isShuttingDown = true;
                gridState.isModalOpen = true;
                gridState.isShutdownConfirmModal = false;
                gridState.modalTitle = "SHUTDOWN";
                gridState.modalMessage = "good bye...";
                gridState.executeHalt();
                return;
            } else {
                // Any other pad cancels confirmation modal
                gridState.isModalOpen = false;
                gridState.isShutdownConfirmModal = false;
                gridState.modalMessage = "";
                return;
            }
        }

        // Row 0: View Navigation, Mute, Rand, Revert, P1 (Play), P2 (Shift), P3 (Shutdown)
        if (row == 0) {
            if (col == 6 || col == 7) return; // MUTE or RAND modifier pad press
            if (col == 8) {
                revertUsedWithSynth = false;
                return; // REVERT modifier pad press
            }
            if (col == 9) { // P1: Play
                if (gridState.isShiftPressed || gridState.pads[10][0].pressed) {
                    studio.seq.isPlaying = !studio.seq.isPlaying;
                    gridState.isModalOpen = false;
                    gridState.isShutdownConfirmModal = false;
                } else {
                    gridState.isModalOpen = true;
                    gridState.isAutoCloseModal = true;
                    gridState.isShutdownConfirmModal = false;
                    gridState.modalTitle = "PLAY CONTROL";
                    gridState.modalMessage = "press shift + pad play/stop";
                    gridState.modalOpenTimeMs = getCurrentTimeMs();
                }
                return;
            }
            if (col == 10) { // P2: Shift
                gridState.isShiftPressed = true;
                updateActiveViewEncoders();
                return;
            }
            if (col == 11) { // P3: Shutdown
                if (gridState.isShiftPressed || gridState.pads[10][0].pressed) {
                    gridState.isModalOpen = true;
                    gridState.isAutoCloseModal = false;
                    gridState.isShutdownConfirmModal = true;
                    gridState.modalTitle = "SHUTDOWN CONFIRMATION";
                    gridState.modalMessage = "press shutdown again to confirm, else press any other pad to cancel.";
                } else {
                    gridState.isModalOpen = true;
                    gridState.isAutoCloseModal = true;
                    gridState.isShutdownConfirmModal = false;
                    gridState.modalTitle = "SHUTDOWN";
                    gridState.modalMessage = "press shift + pad shutdown to switch off...";
                    gridState.modalOpenTimeMs = getCurrentTimeMs();
                }
                return;
            }
            if (col <= 5) {
                // If MUTE modifier pad (col 6) is held, toggle mute for the pressed track
                if (gridState.pads[6][0].pressed) {
                    if (col == 0) gridState.isKickMuted = !gridState.isKickMuted;
                    else if (col == 1) gridState.isSynth1Muted = !gridState.isSynth1Muted;
                    else if (col == 2) gridState.isSynth2Muted = !gridState.isSynth2Muted;
                    else if (col == 3) gridState.isChaosMuted = !gridState.isChaosMuted;
                    return;
                }
                // If RAND modifier pad (col 7) is held, randomize target engine params!
                if (gridState.pads[7][0].pressed) {
                    if (col == 0) randomizeEngine(studio.kick, kickParamBackup, 0);
                    else if (col == 1) {
                        if (studio.synth1EngineIdx == 0) randomizeEngine(studio.synth1, synth1ParamBackup, 1);
                        else randomizeEngine(studio.impactSuper, synth1ParamBackup, 1);
                    }
                    else if (col == 2) randomizeEngine(studio.synth2, synth2ParamBackup, 2);
                    else if (col == 3) randomizeEngine(studio.chaos, chaosParamBackup, 3);
                    updateActiveViewEncoders();
                    return;
                }
                // If REVERT modifier pad (col 8) is held, revert target engine params!
                if (gridState.pads[8][0].pressed) {
                    revertUsedWithSynth = true;
                    if (col == 0) revertEngine(studio.kick, kickParamBackup);
                    else if (col == 1) {
                        if (studio.synth1EngineIdx == 0) revertEngine(studio.synth1, synth1ParamBackup);
                        else revertEngine(studio.impactSuper, synth1ParamBackup);
                    }
                    else if (col == 2) revertEngine(studio.synth2, synth2ParamBackup);
                    else if (col == 3) revertEngine(studio.chaos, chaosParamBackup);
                    updateActiveViewEncoders();
                    return;
                }
                if (col == 0) gridState.activeView = VIEW_KICK;
                else if (col == 1) gridState.activeView = VIEW_SYNTH1;
                else if (col == 2) gridState.activeView = VIEW_SYNTH2;
                else if (col == 3) gridState.activeView = VIEW_CHAOS;
                else if (col == 4) gridState.activeView = VIEW_MASTER;
                else if (col == 5) gridState.activeView = VIEW_SEQUENCER;
                updateActiveViewEncoders();
                return;
            }
            return;
        }

        // Row 3: Performance Row
        if (row == 3) {
            processPerformancePadState();
            return;
        }

        // Rows 1 & 2: Performance & Scatter Pads (same for all views)
        if (row == 1 || row == 2) {
            processPerformancePadState();
            return;
        }
    } else {
        if (row == 0 && col == 10) {
            gridState.isShiftPressed = gridState.pads[10][0].pressed;
            updateActiveViewEncoders();
        }
        // Handle release of REVERT pad on Row 0 (col 8)
        if (row == 0 && col == 8) {
            if (!revertUsedWithSynth && lastRandomizedEngine != -1) {
                if (lastRandomizedEngine == 0) revertEngine(studio.kick, kickParamBackup);
                else if (lastRandomizedEngine == 1) {
                    if (studio.synth1EngineIdx == 0) revertEngine(studio.synth1, synth1ParamBackup);
                    else revertEngine(studio.impactSuper, synth1ParamBackup);
                }
                else if (lastRandomizedEngine == 2) revertEngine(studio.synth2, synth2ParamBackup);
                else if (lastRandomizedEngine == 3) revertEngine(studio.chaos, chaosParamBackup);
                updateActiveViewEncoders();
            }
            revertUsedWithSynth = false;
        }
        processPerformancePadState();
    }
}

inline void renderEncoderGrid(Draw& d, int x, int y, int w, int h) {
    int cardW = w / ENCODER_COLS;
    int cardH = h / ENCODER_ROWS;

    for (int r = 0; r < ENCODER_ROWS; ++r) {
        for (int c = 0; c < ENCODER_COLS; ++c) {
            int idx = r * ENCODER_COLS + c;
            int cx = x + c * cardW;
            int cy = y + r * cardH;

            auto& paramObj = gridState.encoders[idx];
            Color defaultBg = lighten(d.styles.colors.quaternary, 0.2);
            Color bgColor = (gridState.encoderBgColors[idx].a != 0) ? gridState.encoderBgColors[idx] : defaultBg;
            Color pColor = gridState.encoderColors[idx];

            UiParams::param(d, paramObj, cardW, w, cx, cy, bgColor, pColor);
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
            p.active = false;

            // Row 0: Views, Mute, Rand, Revert, P1 (Play), P2 (Shift), P3 (Shutdown)
            if (r == 0) {
                if (c == 0) { p.label = "Kick"; p.color = Color { 0, 195, 255, 255 }; if (gridState.activeView == VIEW_KICK) p.active = true; }
                else if (c == 1) { p.label = "Synth1"; p.color = Color { 0, 240, 190, 255 }; if (gridState.activeView == VIEW_SYNTH1) p.active = true; }
                else if (c == 2) { p.label = "Synth2"; p.color = Color { 215, 125, 255, 255 }; if (gridState.activeView == VIEW_SYNTH2) p.active = true; }
                else if (c == 3) { p.label = "Chaos"; p.color = Color { 255, 45, 85, 255 }; if (gridState.activeView == VIEW_CHAOS) p.active = true; }
                else if (c == 4) { p.label = "Master"; p.color = Color { 255, 215, 0, 255 }; if (gridState.activeView == VIEW_MASTER) p.active = true; }
                else if (c == 5) { p.label = "Seq"; p.color = Color { 60, 220, 100, 255 }; if (gridState.activeView == VIEW_SEQUENCER) p.active = true; }
                else if (c == 6) { p.label = "MUTE"; p.color = gridState.pads[6][0].pressed ? Color { 255, 60, 60, 255 } : Color { 160, 40, 40, 255 }; p.active = gridState.pads[6][0].pressed; }
                else if (c == 7) { p.label = "RAND"; p.color = gridState.pads[7][0].pressed ? Color { 255, 180, 0, 255 } : Color { 180, 120, 0, 255 }; p.active = gridState.pads[7][0].pressed; }
                else if (c == 8) { p.label = "&icon::revert::filled"; p.color = gridState.pads[8][0].pressed ? Color { 0, 220, 255, 255 } : Color { 0, 140, 180, 255 }; p.active = gridState.pads[8][0].pressed; }
                else if (c == 9) { p.label = studio.seq.isPlaying ? "&icon::stop::filled" : "&icon::play::filled"; p.color = studio.seq.isPlaying ? Color { 60, 220, 100, 255 } : Color { 220, 60, 60, 255 }; p.active = studio.seq.isPlaying; }
                else if (c == 10) { p.label = "Shift"; p.color = gridState.isShiftPressed ? Color { 255, 255, 255, 255 } : Color { 140, 150, 170, 255 }; p.active = gridState.isShiftPressed; }
                else if (c == 11) { p.label = "&icon::shutdown"; p.color = gridState.pads[11][0].pressed ? Color { 255, 80, 80, 255 } : Color { 200, 50, 50, 255 }; p.active = gridState.pads[11][0].pressed; }
            }

            // Rows 1 & 2: Performance & Scatter Pads (same for all views)
            if (r == 1 || r == 2) {
                if (c == 0) {
                    if (r == 1) { // RndBar (under Kick pad)
                        p.label = "RndBar";
                        p.color = Color { 0, 195, 255, 255 };
                        p.active = (gridState.isLatchedRndBar || gridState.isPressedRndBar);
                    } else if (r == 2) { // Repeat (over Break pad)
                        p.label = "Repeat";
                        p.color = Color { 0, 195, 255, 255 };
                        p.active = (gridState.isLatchedS || gridState.isPressedS);
                    }
                } else if (c == 1) {
                    if (r == 1) { // +2sm (on right of RndBar)
                        p.label = "+2sm";
                        p.color = Color { 0, 195, 255, 255 };
                        p.active = (gridState.isLatchedPlus2sm || gridState.isPressedPlus2sm);
                    } else if (r == 2) { // -2sm (on right of Repeat)
                        p.label = "-2sm";
                        p.color = Color { 0, 195, 255, 255 };
                        p.active = (gridState.isLatchedMinus2sm || gridState.isPressedMinus2sm);
                    }
                } else {
                    const auto& def = (r == 1) ? scatterRow1[c - 2] : scatterRow2[c - 2];
                    p.label = def.label;
                    p.color = def.color;
                    p.active = (gridState.isLatchedScatter[c][r] || gridState.pads[c][r].pressed);
                }
            }

            // Row 3: Performance Row
            if (r == 3) {
                if (c == 0) { p.label = "Break"; p.color = Color { 0, 195, 255, 255 }; p.active = (gridState.isLatchedA || gridState.isPressedA); }
                else if (c == 1) { p.label = "Drop"; p.color = Color { 0, 195, 255, 255 }; p.active = (gridState.isLatchedSubDrop || gridState.isPressedSubDrop); }
                else if (c == 11) { p.label = "Hold"; p.color = Color { 200, 200, 220, 255 }; p.active = gridState.pads[11][3].pressed; }
                else if (c >= 2 && c <= 10) {
                    const auto& def = scatterRow3[c];
                    p.label = def.label;
                    p.color = def.color;
                    p.active = (gridState.isLatchedScatter[c][3] || gridState.pads[c][3].pressed);
                } else { p.label = ""; p.color = Color { 30, 35, 45, 255 }; }
            }

            // Draw Pad (zicGridV2 design style)
            Color bg = p.color;
            bool isSelected = (r == 0 && c <= 5 && p.active);

            if (!p.pressed && !isSelected && !p.active) {
                bg.r = (uint8_t)(bg.r * 0.40f);
                bg.g = (uint8_t)(bg.g * 0.40f);
                bg.b = (uint8_t)(bg.b * 0.40f);
            }
            if (p.pressed) {
                bg = { 255, 255, 255, 255 };
            }

            int px = x + c * colW;
            int py = y + r * rowH;

            d.filledRect({ px + 1, py + 1 }, { colW - 2, rowH - 2 }, { .color = bg });

            Color border = (p.pressed || p.active) ? Color { 255, 255, 255, 255 } : Color { (uint8_t)(p.color.r / 2), (uint8_t)(p.color.g / 2), (uint8_t)(p.color.b / 2), 255 };
            d.rect({ px + 1, py + 1 }, { colW - 2, rowH - 2 }, { .color = border });

            if (isSelected) {
                d.filledRect({ px + 2, py + rowH - 4 }, { colW - 4, 3 }, { .color = { 255, 255, 255, 255 } });
                d.filledCircle({ px + colW - 5, py + 5 }, 2, { .color = { 255, 255, 255, 255 } });
            }

            bool isTrackMuted = (r == 0) && (
                (c == 0 && gridState.isKickMuted) ||
                (c == 1 && gridState.isSynth1Muted) ||
                (c == 2 && gridState.isSynth2Muted) ||
                (c == 3 && gridState.isChaosMuted)
            );

            if (isTrackMuted) {
                Icon icon(d);
                Color muteIconCol = p.pressed ? Color { 0, 0, 0, 255 } : Color { 255, 255, 255, 255 };
                icon.render("&icon::mute", { px + 3, py + 3 }, { 9, 9 }, muteIconCol);
            }

            if (!p.label.empty()) {
                Color textCol = p.pressed ? Color { 0, 0, 0, 255 } : getContrastTextColor(bg);
                if (p.label[0] == '&') {
                    Icon icon(d);
                    int iconSize = 12;
                    icon.render(p.label, { px + (colW - iconSize) / 2, py + (rowH - iconSize) / 2 }, iconSize, textCol);
                } else {
                    d.textCentered({ px + colW / 2, py + rowH / 2 - 4 }, p.label, 8, { .color = textCol, .font = &PoppinsLight_8 });
                }
            }
        }
    }
}

inline bool drawUI(Draw& d, int w, int h, bool& needFullRedraw) {
    if (gridState.isModalOpen && gridState.isAutoCloseModal) {
        if (getCurrentTimeMs() - gridState.modalOpenTimeMs >= 2000) {
            gridState.isModalOpen = false;
            gridState.isAutoCloseModal = false;
            gridState.modalMessage = "";
        }
    }

    updateActiveViewEncoders();

    d.filledRect({ 0, 0 }, { w, h }, { .color = d.styles.colors.background });

    int margin = 4;
    int usableW = w - margin * 2;

    // 1. TOP: 12 Encoder Grid (3 rows x 4 cols = 108px matching zicGridV2)
    int encoderH = 3 * UiParams::ROW_H;
    renderEncoderGrid(d, margin, margin, usableW, encoderH);

    // 2. BOTTOM: 48-Pad Grid (4 rows x 12 cols)
    int padGridH = 145;
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
        case VIEW_CHAOS: chaosView.render(d, margin, currentY, usableW, availableMiddleH); break;
    }

    renderPadGrid(d, margin, padGridY, usableW, padGridH);

    if (gridState.isModalOpen || gridState.isShuttingDown) {
        std::string msg = gridState.modalMessage;
        bool isMultiLine = (msg.length() > 45);

        int boxW = isMultiLine ? 380 : 320;
        int boxH = isMultiLine ? 46 : 30;
        int boxX = (w - boxW) / 2;
        int boxY = currentY + (availableMiddleH - boxH) / 2;

        Color borderColor = (gridState.isShutdownConfirmModal || gridState.isShuttingDown)
                            ? Color { 255, 60, 60, 255 }
                            : Color { 0, 195, 255, 255 };
        Color boxBg = Color { 14, 18, 26, 200 };

        d.filledRect({ boxX, boxY }, { boxW, boxH }, { .color = boxBg });
        d.rect({ boxX, boxY }, { boxW, boxH }, { .color = borderColor });

        if (isMultiLine) {
            size_t spaceIdx = msg.find("confirm, ");
            if (spaceIdx != std::string::npos) {
                std::string line1 = msg.substr(0, spaceIdx + 8);
                std::string line2 = msg.substr(spaceIdx + 9);
                d.textCentered({ w / 2, boxY + 12 }, line1, 8, { .color = Color { 240, 248, 255, 255 }, .font = &PoppinsLight_8 });
                d.textCentered({ w / 2, boxY + 28 }, line2, 8, { .color = Color { 240, 248, 255, 255 }, .font = &PoppinsLight_8 });
            } else {
                d.textCentered({ w / 2, boxY + 19 }, msg, 8, { .color = Color { 240, 248, 255, 255 }, .font = &PoppinsLight_8 });
            }
        } else {
            d.textCentered({ w / 2, boxY + 11 }, msg, 8, { .color = Color { 240, 248, 255, 255 }, .font = &PoppinsLight_8 });
        }
    }

    return true;
}
