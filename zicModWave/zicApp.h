#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "audioEngine.h"

struct StepData {
    bool active = false;
    uint8_t note = 48;   // MIDI note C3
    uint8_t vel = 100;   // Velocity 0..127
    uint8_t len = 1;     // Duration in steps
};

struct PatternData {
    StepData steps[32];
};

enum PotIndex {
    POT_WAVE = 0,    // Row 1 Left:  Waveform Morph (A11 - Pin 11)
    POT_CRUSH_FM,    // Row 1 Right: Crsh / FM Centered Pot (A10 - Pin 10)
    POT_CUTOFF,      // Row 2 Left:  Filter Cutoff (A7 - Pin 7)
    POT_RESONANCE,   // Row 2 Mid:   Filter Resonance (A8 - Pin 8)
    POT_FILT_MORPH,  // Row 2 Right: Filter Morph LP->BP->HP (A9 - Pin 9)
    POT_MOD_DEPTH,   // Row 3 Left:  Mod Depth (A4 - Pin 4)
    POT_MOD_SPEED,   // Row 3 Mid:   Mod Speed (A5 - Pin 5)
    POT_DLY_SEND,    // Row 3 Right: Delay Send (A6 - Pin 6)
    NUM_POTS
};

enum ViewMode {
    VIEW_3D_SYNTH = 0,
    VIEW_SEQ_GRID,
    VIEW_STEP_EDIT
};

enum SubmenuMode {
    SUBMENU_NONE = 0,
    SUBMENU_SEQ_MAIN,
    SUBMENU_SEQ_SELECT,
    SUBMENU_SEQ_GENERATE
};

class ZicApp {
public:
    WaveEngine engine;

    // 8 Hardware Pot Values
    float potValues[NUM_POTS] = { 0.3f, 0.5f, 0.4f, 0.3f, 0.0f, 0.5f, 0.5f, 0.2f };
    int32_t potOverlayTimer = 0;
    int lastMovedPotIndex = 0;

    // Menu State & Overlay Timers
    int currentMenuItem = 0;
    bool isEditing = false;
    int32_t menuOverlayTimer = 0;

    // View & Submenu States
    ViewMode currentView = VIEW_3D_SYNTH;
    SubmenuMode currentSubmenu = SUBMENU_NONE;

    // Sequencer 100 Pattern Storage
    PatternData* patterns = nullptr;
    int activePatternIdx = 0;   // Loaded active pattern (0..99)
    int selectedPatternIdx = 0; // Preview pattern selection in menu (0..99)

    // Generator Parameters
    int genDensity = 60;   // 0..100% density
    int genPitchRange = 12; // 0..24 semitones range
    int genStyle = 0;       // 0: Bass, 1: Arp, 2: Random

    // Sequencer View Step Selection
    int selectedStepIdx = 0; // 0..31
    int stepEditParamIdx = 0; // 0: Active, 1: Note, 2: Vel, 3: Len, 4: Back
    bool isEditingStepParam = false;

    // Timing & Clock
    bool isExternalClock = false;
    uint32_t lastMidiClockTimeMs = 0;
    bool isPlaying = true;
    bool isDirty = true;

    ZicApp(float sr = 44100.0f)
        : engine(sr)
    {
        patterns = (PatternData*)calloc(100, sizeof(PatternData));
        initDefaultPatterns();
        syncPotsToEngine();
    }

    ~ZicApp()
    {
        if (patterns) {
            free(patterns);
            patterns = nullptr;
        }
    }

    void initDefaultPatterns()
    {
        // Zero all 100 patterns
        for (int p = 0; p < 100; ++p) {
            for (int s = 0; s < 32; ++s) {
                patterns[p].steps[s].active = false;
                patterns[p].steps[s].note = 36 + (s % 12);
                patterns[p].steps[s].vel = 90;
                patterns[p].steps[s].len = 1;
            }
        }

        // Initialize Pattern 0 with default sequence
        const int notes[32] = {
            36,36,48,36, 48,36,43,36, 36,48,36,39, 43,36,41,38,
            36,36,48,43, 36,48,36,36, 43,39,36,41, 48,43,36,41
        };
        const bool active[32] = {
            1,0,1,0, 1,0,1,0, 1,1,0,0, 1,0,1,0,
            1,0,0,1, 1,0,1,0, 1,0,1,0, 1,1,0,1
        };

        for (int i = 0; i < 32; ++i) {
            patterns[0].steps[i].active = active[i];
            patterns[0].steps[i].note = (uint8_t)notes[i];
            patterns[0].steps[i].vel = active[i] ? 100 : 0;
            patterns[0].steps[i].len = 1;
        }
    }

    void generatePattern(int pIdx, int densityPct, int pitchRange, int style)
    {
        if (pIdx < 0 || pIdx >= 100) return;
        for (int i = 0; i < 32; ++i) {
            bool act = (rand() % 100) < densityPct;
            patterns[pIdx].steps[i].active = act;
            if (act) {
                int nOffset = (rand() % (pitchRange + 1));
                patterns[pIdx].steps[i].note = 36 + nOffset;
                patterns[pIdx].steps[i].vel = 80 + (rand() % 40);
                patterns[pIdx].steps[i].len = 1 + (rand() % 2);
            } else {
                patterns[pIdx].steps[i].vel = 0;
            }
        }
    }

    void syncPotsToEngine()
    {
        engine.waveform.set(potValues[POT_WAVE]);
        engine.crushFm.set(potValues[POT_CRUSH_FM] * 100.0f);
        engine.cutoff.set(0.02f + potValues[POT_CUTOFF] * 0.96f);
        engine.resonance.set(potValues[POT_RESONANCE] * 0.95f);
        engine.filterMorph.set(potValues[POT_FILT_MORPH]);
        engine.modDepth.set(potValues[POT_MOD_DEPTH] * 200.0f - 100.0f);
        engine.modSpeed.set(potValues[POT_MOD_SPEED] * 100.0f);
        engine.delaySend.set(potValues[POT_DLY_SEND] * 100.0f);
    }

    void applyPotValue(PotIndex pot, float normVal)
    {
        if (pot < 0 || pot >= NUM_POTS) return;
        normVal = std::clamp(normVal, 0.0f, 1.0f);
        potValues[pot] = normVal;
        lastMovedPotIndex = pot;
        potOverlayTimer = 90; // ~1.5 sec toast overlay timer at 60 FPS

        switch (pot) {
            case POT_WAVE:
                engine.waveform.set(normVal);
                break;
            case POT_CRUSH_FM:
                engine.crushFm.set(normVal * 100.0f);
                break;
            case POT_CUTOFF:
                engine.cutoff.set(0.02f + normVal * 0.96f);
                break;
            case POT_RESONANCE:
                engine.resonance.set(normVal * 0.95f);
                break;
            case POT_FILT_MORPH:
                engine.filterMorph.set(normVal);
                break;
            case POT_MOD_DEPTH:
                engine.modDepth.set(normVal * 200.0f - 100.0f);
                break;
            case POT_MOD_SPEED:
                engine.modSpeed.set(normVal * 100.0f);
                break;
            case POT_DLY_SEND:
                engine.delaySend.set(normVal * 100.0f);
                break;
            default:
                break;
        }
    }

    const char* getPotName(PotIndex pot) const
    {
        switch (pot) {
            case POT_WAVE:       return "Wave";
            case POT_CRUSH_FM: {
                float noiseFade = std::clamp((engine.waveform.value - 0.50f) / 0.30f, 0.0f, 1.0f);
                return (noiseFade < 0.2f) ? "FM Depth" : ((noiseFade > 0.8f) ? "Bitcrush" : "FM + Crush");
            }
            case POT_CUTOFF:     return "Cutoff";
            case POT_RESONANCE:  return "Reso";
            case POT_FILT_MORPH: return "Filt Morph";
            case POT_MOD_DEPTH:  return "Mod Depth";
            case POT_MOD_SPEED:  return "Mod Speed";
            case POT_DLY_SEND:   return "Dly Send";
            default:             return "";
        }
    }

    void getPotFormattedValue(PotIndex pot, char* buf, size_t bufSize) const
    {
        switch (pot) {
            case POT_WAVE: {
                float wf = engine.waveform.value;
                if (wf < 0.333f) {
                    snprintf(buf, bufSize, "Tri->Saw (%.0f%%)", wf * 300.0f);
                } else if (wf < 0.666f) {
                    snprintf(buf, bufSize, "Saw->Sq (%.0f%%)", (wf - 0.333f) * 300.0f);
                } else {
                    snprintf(buf, bufSize, "Sq->Noise (%.0f%%)", (wf - 0.666f) * 300.0f);
                }
                break;
            }
            case POT_CRUSH_FM: {
                float val = engine.crushFm.value;
                if (val <= 0.5f) {
                    snprintf(buf, bufSize, "Clean");
                } else {
                    float noiseFade = std::clamp((engine.waveform.value - 0.50f) / 0.30f, 0.0f, 1.0f);
                    if (noiseFade < 0.2f) {
                        snprintf(buf, bufSize, "FM %.0f%%", val);
                    } else if (noiseFade > 0.8f) {
                        snprintf(buf, bufSize, "Crush %.0f%%", val);
                    } else {
                        snprintf(buf, bufSize, "FM+Crush %.0f%%", val);
                    }
                }
                break;
            }
            case POT_CUTOFF:
                snprintf(buf, bufSize, "%.0f %%", engine.cutoff.value * 100.0f);
                break;
            case POT_RESONANCE:
                snprintf(buf, bufSize, "%.0f %%", engine.resonance.value * 100.0f);
                break;
            case POT_FILT_MORPH: {
                float fm = engine.filterMorph.value;
                if (fm < 0.5f) {
                    snprintf(buf, bufSize, "LP->BP (%.0f%%)", fm * 200.0f);
                } else {
                    snprintf(buf, bufSize, "BP->HP (%.0f%%)", (fm - 0.5f) * 200.0f);
                }
                break;
            }
            case POT_MOD_DEPTH:
                snprintf(buf, bufSize, "%+.0f %%", engine.modDepth.value);
                break;
            case POT_MOD_SPEED:
                snprintf(buf, bufSize, "%.0f %%", engine.modSpeed.value);
                break;
            case POT_DLY_SEND:
                snprintf(buf, bufSize, "%.0f %%", engine.delaySend.value);
                break;
            default:
                snprintf(buf, bufSize, "0");
                break;
        }
    }

    static constexpr int NUM_MENU_ITEMS = 9;

    const char* getMenuItemName(int index) const
    {
        switch (index) {
            case 0: return "Mod Type";
            case 1: return "Pitch";
            case 2: return "Release";
            case 3: return "Volume";
            case 4: return "BPM";
            case 5: return "Delay Time";
            case 6: return "Delay FB";
            case 7: return "PLAY / STOP";
            case 8: return "Sequencer...";
            default: return "";
        }
    }

    void getMenuItemFormattedValue(int index, char* buf, size_t bufSize) const
    {
        switch (index) {
            case 0:
                snprintf(buf, bufSize, "%s", engine.modTypeNameDisplay);
                break;
            case 1: {
                int noteNum = (int)engine.pitch.value;
                static const char* NOTE_NAMES[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
                int oct = (noteNum / 12) - 1;
                snprintf(buf, bufSize, "%s%d (%d)", NOTE_NAMES[noteNum % 12], oct, noteNum);
                break;
            }
            case 2:
                snprintf(buf, bufSize, "%.0f ms", engine.release.value);
                break;
            case 3:
                snprintf(buf, bufSize, "%.0f %%", engine.masterVol.value);
                break;
            case 4:
                if (isExternalClock) {
                    snprintf(buf, bufSize, "MIDI SYNC");
                } else {
                    snprintf(buf, bufSize, "%.0f BPM", engine.bpmParam.value);
                }
                break;
            case 5:
                snprintf(buf, bufSize, "%.0f ms", engine.delayTimeMs);
                break;
            case 6:
                snprintf(buf, bufSize, "%.0f %%", engine.delayFeedback * 100.0f);
                break;
            case 7:
                snprintf(buf, bufSize, "%s", isPlaying ? "PLAYING" : "STOPPED");
                break;
            case 8:
                snprintf(buf, bufSize, "P%d Active", activePatternIdx + 1);
                break;
            default:
                snprintf(buf, bufSize, "-");
                break;
        }
    }

    void handleEncoderTurn(int dir)
    {
        isDirty = true;
        if (potOverlayTimer > 0) potOverlayTimer = 0;

        if (currentView == VIEW_3D_SYNTH) {
            if (currentSubmenu == SUBMENU_NONE) {
                if (menuOverlayTimer == 0) {
                    menuOverlayTimer = 90; // Re-open menu at current item on first tick
                    return;
                }
                menuOverlayTimer = 90;

                if (!isEditing) {
                    currentMenuItem = (currentMenuItem + dir + NUM_MENU_ITEMS) % NUM_MENU_ITEMS;
                } else {
                    // Editing selected parameter
                    switch (currentMenuItem) {
                        case 0: { // Mod Type
                            float v = std::clamp(engine.modType.value + dir, 0.0f, 15.0f);
                            engine.modType.set(v);
                            break;
                        }
                        case 1: { // Pitch
                            float v = std::clamp(engine.pitch.value + dir, 24.0f, 72.0f);
                            engine.pitch.set(v);
                            break;
                        }
                        case 2: { // Release
                            float v = std::clamp(engine.release.value + dir * 10.0f, 10.0f, 2000.0f);
                            engine.release.set(v);
                            break;
                        }
                        case 3: // Volume
                            engine.masterVol.set(std::clamp(engine.masterVol.value + dir * 2.0f, 0.0f, 100.0f));
                            break;
                        case 4: // BPM
                            engine.bpmParam.set(std::clamp(engine.bpmParam.value + dir * 1.0f, 40.0f, 240.0f));
                            break;
                        case 5: // Delay Time
                            engine.delayTimeMs = std::clamp(engine.delayTimeMs + dir * 10.0f, 50.0f, 500.0f);
                            break;
                        case 6: // Delay Feedback
                            engine.delayFeedback = std::clamp(engine.delayFeedback + dir * 0.05f, 0.0f, 0.90f);
                            break;
                        case 7: // PLAY / STOP
                            isPlaying = !isPlaying;
                            engine.isPlaying = isPlaying;
                            if (isPlaying) engine.resetClock();
                            break;
                        default:
                            break;
                    }
                }
            } else if (currentSubmenu == SUBMENU_SEQ_MAIN) {
                selectedPatternIdx = (selectedPatternIdx + dir + 100) % 100;
            } else if (currentSubmenu == SUBMENU_SEQ_SELECT) {
                selectedPatternIdx = (selectedPatternIdx + dir + 100) % 100;
            } else if (currentSubmenu == SUBMENU_SEQ_GENERATE) {
                genDensity = std::clamp(genDensity + dir * 5, 10, 100);
                generatePattern(activePatternIdx, genDensity, genPitchRange, genStyle);
            }
        } else if (currentView == VIEW_SEQ_GRID) {
            if (!isEditingStepParam) {
                selectedStepIdx = (selectedStepIdx + dir + 32) % 32;
            } else {
                StepData& s = patterns[activePatternIdx].steps[selectedStepIdx];
                switch (stepEditParamIdx) {
                    case 0: s.active = !s.active; break;
                    case 1: s.note = std::clamp(s.note + dir, 24, 84); break;
                    case 2: s.vel = std::clamp(s.vel + dir * 5, 0, 127); break;
                    case 3: s.len = std::clamp(s.len + dir, 1, 4); break;
                    default: break;
                }
            }
        }
    }

    void handleEncoderClick()
    {
        isDirty = true;
        if (potOverlayTimer > 0) {
            potOverlayTimer = 0;
            return;
        }

        if (currentView == VIEW_3D_SYNTH) {
            if (currentSubmenu == SUBMENU_NONE) {
                if (menuOverlayTimer == 0) {
                    menuOverlayTimer = 90;
                    return;
                }
                if (currentMenuItem == 7) { // PLAY / STOP
                    isPlaying = !isPlaying;
                    engine.isPlaying = isPlaying;
                    if (isPlaying) engine.resetClock();
                } else if (currentMenuItem == 8) { // Sequencer...
                    currentSubmenu = SUBMENU_SEQ_MAIN;
                    isEditing = false;
                } else {
                    isEditing = !isEditing;
                }
            } else if (currentSubmenu == SUBMENU_SEQ_MAIN) {
                // Sequencer Submenu Options
                if (selectedPatternIdx == 0) {
                    currentSubmenu = SUBMENU_SEQ_SELECT;
                } else if (selectedPatternIdx == 1) {
                    currentSubmenu = SUBMENU_SEQ_GENERATE;
                } else {
                    currentView = VIEW_SEQ_GRID;
                    currentSubmenu = SUBMENU_NONE;
                }
            } else if (currentSubmenu == SUBMENU_SEQ_SELECT) {
                // Confirm & load pattern
                activePatternIdx = selectedPatternIdx;
                currentSubmenu = SUBMENU_NONE;
                menuOverlayTimer = 0;
            } else if (currentSubmenu == SUBMENU_SEQ_GENERATE) {
                currentSubmenu = SUBMENU_NONE;
                menuOverlayTimer = 0;
            }
        } else if (currentView == VIEW_SEQ_GRID) {
            if (!isEditingStepParam) {
                isEditingStepParam = true;
                stepEditParamIdx = 0;
            } else {
                stepEditParamIdx++;
                if (stepEditParamIdx >= 4) {
                    isEditingStepParam = false;
                    stepEditParamIdx = 0;
                }
            }
        }
    }

    void exitToMainView()
    {
        currentView = VIEW_3D_SYNTH;
        currentSubmenu = SUBMENU_NONE;
        isEditing = false;
        isEditingStepParam = false;
        menuOverlayTimer = 0;
    }

    void updateMidiClockTimeout(uint32_t currentMs)
    {
        if (isExternalClock && (currentMs - lastMidiClockTimeMs > 500)) {
            isExternalClock = false;
            engine.isExternalClock = false;
        }
    }

    void handleMidiByte(uint8_t byte, uint32_t currentMs)
    {
        if (byte == 0xF8) {
            isExternalClock = true;
            engine.isExternalClock = true;
            lastMidiClockTimeMs = currentMs;
            if (isPlaying) {
                engine.onMidiClockPulse();
            }
        } else if (byte == 0xFA) {
            isPlaying = true;
            engine.isPlaying = true;
            isExternalClock = true;
            engine.isExternalClock = true;
            lastMidiClockTimeMs = currentMs;
            engine.resetClock();
        } else if (byte == 0xFC) {
            isPlaying = false;
            engine.isPlaying = false;
        } else if ((byte & 0xF0) == 0x90) {
            engine.synth.noteOn(byte, 1.0f);
        }
    }
};
