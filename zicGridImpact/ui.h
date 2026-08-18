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

inline void processPerformancePadState() {
    gridState.isPressedA = gridState.pads[0][3].pressed;      // Break (Col 0)
    gridState.isPressedS = gridState.pads[1][3].pressed;      // Repeat (Col 1)
    gridState.isPressedRndBar = gridState.pads[2][3].pressed; // RndBar (Col 2)
    gridState.isPressedZ = gridState.pads[4][3].pressed;      // Crunch (Col 4)
    gridState.isPressedX = gridState.pads[5][3].pressed;      // Drive (Col 5)
    gridState.isPressedC = gridState.pads[6][3].pressed;      // Dist (Col 6)
    gridState.isPressedV = gridState.pads[7][3].pressed;      // Acid (Col 7)

    // Latch processing (Pad 3, Row 3 -> Hold)
    if (gridState.pads[3][3].pressed) {
        if (gridState.isPressedA) gridState.isLatchedA = !gridState.isLatchedA;
        if (gridState.isPressedS) gridState.isLatchedS = !gridState.isLatchedS;
        if (gridState.isPressedRndBar) gridState.isLatchedRndBar = !gridState.isLatchedRndBar;
        if (gridState.isPressedZ) gridState.isLatchedZ = !gridState.isLatchedZ;
        if (gridState.isPressedX) gridState.isLatchedX = !gridState.isLatchedX;
        if (gridState.isPressedC) gridState.isLatchedC = !gridState.isLatchedC;
        if (gridState.isPressedV) gridState.isLatchedV = !gridState.isLatchedV;
    }

    studio.kick.isBodyMuted = gridState.isLatchedA || gridState.isPressedA;
    studio.seq.isKickRepeatActive = gridState.isLatchedS || gridState.isPressedS;
    studio.seq.isKickRandomBarActive = gridState.isLatchedRndBar || gridState.isPressedRndBar;

    studio.scatter.setModeActive(4, gridState.isLatchedZ || gridState.isPressedZ);
    studio.scatter.setModeActive(5, gridState.isLatchedX || gridState.isPressedX);
    studio.scatter.setModeActive(2, gridState.isLatchedC || gridState.isPressedC);
    studio.scatter.setModeActive(6, gridState.isLatchedV || gridState.isPressedV);
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
                    gridState.modalMessage = "press shift + pad to play/stop";
                    gridState.modalOpenTimeMs = getCurrentTimeMs();
                }
                return;
            }
            if (col == 10) { // P2: Shift
                gridState.isShiftPressed = true;
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
                    else if (col == 1) randomizeEngine(studio.synth1, synth1ParamBackup, 1);
                    else if (col == 2) randomizeEngine(studio.synth2, synth2ParamBackup, 2);
                    else if (col == 3) randomizeEngine(studio.chaos, chaosParamBackup, 3);
                    updateActiveViewEncoders();
                    return;
                }
                // If REVERT modifier pad (col 8) is held, revert target engine params!
                if (gridState.pads[8][0].pressed) {
                    revertUsedWithSynth = true;
                    if (col == 0) revertEngine(studio.kick, kickParamBackup);
                    else if (col == 1) revertEngine(studio.synth1, synth1ParamBackup);
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

        // Rows 1 & 2: Contextual Pads (24 pads)
        if (row == 1 || row == 2) {
            int stepIdx = (row - 1) * 12 + col;
            if (gridState.activeView == VIEW_SEQUENCER) {
                if (stepIdx < 64) {
                    studio.seq.kickPattern[stepIdx] = !studio.seq.kickPattern[stepIdx];
                }
            } else if (gridState.activeView == VIEW_KICK) {
                studio.kick.noteOn(36, 1.0f);
            } else if (gridState.activeView == VIEW_SYNTH1) {
                uint8_t note = static_cast<uint8_t>(gridState.currentOctave * 12 + stepIdx);
                studio.synth1.noteOn(note, 0.9f);
            } else if (gridState.activeView == VIEW_SYNTH2) {
                uint8_t note = static_cast<uint8_t>(gridState.currentOctave * 12 + stepIdx);
                studio.synth2.noteOn(note, 0.9f);
            } else if (gridState.activeView == VIEW_CHAOS) {
                uint8_t note = static_cast<uint8_t>(gridState.currentOctave * 12 + stepIdx);
                studio.chaos.noteOn(note, 0.9f);
            }
        }
    } else {
        if (row == 0 && col == 10) {
            gridState.isShiftPressed = gridState.pads[10][0].pressed;
        }
        // Handle release of REVERT pad on Row 0 (col 8)
        if (row == 0 && col == 8) {
            if (!revertUsedWithSynth && lastRandomizedEngine != -1) {
                if (lastRandomizedEngine == 0) revertEngine(studio.kick, kickParamBackup);
                else if (lastRandomizedEngine == 1) revertEngine(studio.synth1, synth1ParamBackup);
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

            // Rows 1 & 2: Contextual Step / Note Pads (24 pads)
            if (r == 1 || r == 2) {
                int padIdx = (r - 1) * 12 + c;
                if (gridState.activeView == VIEW_SEQUENCER) {
                    bool stepOn = studio.seq.kickPattern[padIdx];
                    bool isCurrent = (studio.seq.currentStep == padIdx);
                    p.label = std::to_string(padIdx + 1);
                    if (isCurrent) {
                        p.color = Color { 255, 255, 255, 255 };
                        p.active = true;
                    } else if (stepOn) {
                        p.color = Color { 60, 220, 100, 255 };
                        p.active = true;
                    } else {
                        p.color = Color { 20, 70, 35, 255 };
                        p.active = false;
                    }
                } else if (gridState.activeView == VIEW_KICK) {
                    p.label = "K" + std::to_string(padIdx + 1);
                    p.color = Color { 0, 195, 255, 255 };
                } else if (gridState.activeView == VIEW_SYNTH1) {
                    uint8_t note = static_cast<uint8_t>(gridState.currentOctave * 12 + padIdx);
                    static const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
                    p.label = std::string(noteNames[note % 12]) + std::to_string(note / 12);
                    p.color = Color { 0, 240, 190, 255 };
                } else if (gridState.activeView == VIEW_SYNTH2) {
                    uint8_t note = static_cast<uint8_t>(gridState.currentOctave * 12 + padIdx);
                    static const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
                    p.label = std::string(noteNames[note % 12]) + std::to_string(note / 12);
                    p.color = Color { 215, 125, 255, 255 };
                } else if (gridState.activeView == VIEW_CHAOS) {
                    uint8_t note = static_cast<uint8_t>(gridState.currentOctave * 12 + padIdx);
                    static const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
                    p.label = std::string(noteNames[note % 12]) + std::to_string(note / 12);
                    p.color = Color { 255, 45, 85, 255 };
                } else {
                    p.label = std::to_string(padIdx + 1);
                    p.color = Color { 50, 60, 80, 255 };
                }
            }

            // Row 3: Performance Row
            if (r == 3) {
                if (c == 0) { p.label = "Break"; p.color = Color { 0, 195, 255, 255 }; p.active = (gridState.isLatchedA || gridState.isPressedA); }
                else if (c == 1) { p.label = "Repeat"; p.color = Color { 255, 215, 0, 255 }; p.active = (gridState.isLatchedS || gridState.isPressedS); }
                else if (c == 2) { p.label = "RndBar"; p.color = Color { 255, 140, 0, 255 }; p.active = (gridState.isLatchedRndBar || gridState.isPressedRndBar); }
                else if (c == 3) { p.label = "Hold"; p.color = Color { 200, 200, 220, 255 }; p.active = gridState.pads[3][3].pressed; }
                else if (c == 4) { p.label = "Crunch"; p.color = Color { 0, 220, 255, 255 }; p.active = (gridState.isLatchedZ || gridState.isPressedZ); }
                else if (c == 5) { p.label = "Drive"; p.color = Color { 100, 120, 255, 255 }; p.active = (gridState.isLatchedX || gridState.isPressedX); }
                else if (c == 6) { p.label = "Dist"; p.color = Color { 255, 80, 180, 255 }; p.active = (gridState.isLatchedC || gridState.isPressedC); }
                else if (c == 7) { p.label = "Acid"; p.color = Color { 60, 220, 100, 255 }; p.active = (gridState.isLatchedV || gridState.isPressedV); }
                else { p.label = ""; p.color = Color { 30, 35, 45, 255 }; }
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
        Color boxBg = Color { 14, 18, 26, 245 };

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
