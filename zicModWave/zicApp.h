#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

#include "audioEngine.h"

enum PotIndex {
    POT_WAVE = 0,    // Row 1 Left:  Waveform Morph (A11 - Pin 11)
    POT_CRUSH_FM,    // Row 1 Right: Crsh / FM Centered Pot (A10 - Pin 10)
    POT_CUTOFF,      // Row 2 Left:  Filter Cutoff (A7 - Pin 7)
    POT_RESONANCE,   // Row 2 Mid:   Filter Resonance (A8 - Pin 8)
    POT_RING_MOD,    // Row 2 Right: Ring Modulator (A9 - Pin 9)
    POT_MOD_DEPTH,   // Row 3 Left:  Mod Depth (A4 - Pin 4)
    POT_MOD_SPEED,   // Row 3 Mid:   Mod Speed (A5 - Pin 5)
    POT_DLY_SEND,    // Row 3 Right: Delay Send (A6 - Pin 6)
    NUM_POTS
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

    // Sequencer Rhythm & Arp Parameters
    int rhythmPatternIdx = 0; // 0..5
    int arpModeIdx = 0;       // 0..6

    // Timing & Clock
    bool isExternalClock = false;
    uint32_t lastMidiClockTimeMs = 0;
    bool isPlaying = true;
    bool isDirty = true;

    ZicApp(float sr = 44100.0f)
        : engine(sr)
    {
        syncPotsToEngine();
        engine.updateSequence(rhythmPatternIdx, arpModeIdx);
    }

    ~ZicApp() = default;

    void syncPotsToEngine()
    {
        engine.waveform.set(potValues[POT_WAVE]);
        engine.crushFm.set(potValues[POT_CRUSH_FM] * 100.0f);
        engine.cutoff.set(0.02f + potValues[POT_CUTOFF] * 0.96f);
        engine.resonance.set(potValues[POT_RESONANCE] * 0.95f);
        engine.ringMod.set(potValues[POT_RING_MOD]);
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
            case POT_RING_MOD:
                engine.ringMod.set(normVal);
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
            case POT_RING_MOD:   return "Ring Mod";
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
            case POT_RING_MOD:
                snprintf(buf, bufSize, "%.0f %%", engine.ringMod.value * 100.0f);
                break;
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

    static constexpr int NUM_MENU_ITEMS = 11;

    const char* getMenuItemName(int index) const
    {
        switch (index) {
            case 0: return "Mod Type";
            case 1: return "FM Ratio";
            case 2: return "Pitch";
            case 3: return "Env Amt";
            case 4: return "Volume";
            case 5: return "BPM";
            case 6: return "Rhythm";
            case 7: return "Arp Mode";
            case 8: return "Delay Time";
            case 9: return "Delay FB";
            case 10: return "PLAY / STOP";
            default: return "";
        }
    }

    void getMenuItemFormattedValue(int index, char* buf, size_t bufSize) const
    {
        switch (index) {
            case 0:
                snprintf(buf, bufSize, "%s", engine.modTypeNameDisplay);
                break;
            case 1:
                snprintf(buf, bufSize, "%.1f x", engine.fmRatioParam.value);
                break;
            case 2: {
                int noteNum = (int)engine.pitch.value;
                static const char* NOTE_NAMES[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
                int oct = (noteNum / 12) - 1;
                snprintf(buf, bufSize, "%s%d (%d)", NOTE_NAMES[noteNum % 12], oct, noteNum);
                break;
            }
            case 3:
                snprintf(buf, bufSize, "%.0f %%", engine.envAmt.value * 100.0f);
                break;
            case 4:
                snprintf(buf, bufSize, "%.0f %%", engine.masterVol.value);
                break;
            case 5:
                if (isExternalClock) {
                    snprintf(buf, bufSize, "MIDI SYNC");
                } else {
                    snprintf(buf, bufSize, "%.0f BPM", engine.bpmParam.value);
                }
                break;
            case 6:
                snprintf(buf, bufSize, "%s", WaveEngine::RHYTHM_NAMES[rhythmPatternIdx]);
                break;
            case 7:
                snprintf(buf, bufSize, "%s", WaveEngine::ARP_NAMES[arpModeIdx]);
                break;
            case 8:
                snprintf(buf, bufSize, "%.0f ms", engine.delayTimeMs);
                break;
            case 9:
                snprintf(buf, bufSize, "%.0f %%", engine.delayFeedback * 100.0f);
                break;
            case 10:
                snprintf(buf, bufSize, "%s", isPlaying ? "PLAYING" : "STOPPED");
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

        if (menuOverlayTimer == 0) {
            menuOverlayTimer = 90; // Re-open menu overlay at current item on first turn
            return;
        }
        menuOverlayTimer = 90;

        if (!isEditing) {
            currentMenuItem = (currentMenuItem + dir + NUM_MENU_ITEMS) % NUM_MENU_ITEMS;
        } else {
            // Direct Parameter Editing
            switch (currentMenuItem) {
                case 0: { // Mod Type
                    float v = std::clamp(engine.modType.value + dir, 0.0f, 15.0f);
                    engine.modType.set(v);
                    break;
                }
                case 1: { // FM Ratio
                    float v = std::clamp(engine.fmRatioParam.value + dir * 0.5f, 0.5f, 8.0f);
                    engine.fmRatioParam.set(v);
                    break;
                }
                case 2: { // Pitch
                    float v = std::clamp(engine.pitch.value + dir, 24.0f, 72.0f);
                    engine.pitch.set(v);
                    break;
                }
                case 3: { // Env Amt
                    float v = std::clamp(engine.envAmt.value + dir * 0.05f, 0.0f, 1.0f);
                    engine.envAmt.set(v);
                    break;
                }
                case 4: // Volume
                    engine.masterVol.set(std::clamp(engine.masterVol.value + dir * 2.0f, 0.0f, 100.0f));
                    break;
                case 5: // BPM
                    engine.bpmParam.set(std::clamp(engine.bpmParam.value + dir * 1.0f, 40.0f, 240.0f));
                    break;
                case 6: // Rhythm Pattern
                    rhythmPatternIdx = (rhythmPatternIdx + dir + WaveEngine::TOTAL_RHYTHM_PATTERNS) % WaveEngine::TOTAL_RHYTHM_PATTERNS;
                    engine.updateSequence(rhythmPatternIdx, arpModeIdx);
                    break;
                case 7: // Arp Mode
                    arpModeIdx = (arpModeIdx + dir + WaveEngine::TOTAL_ARP_MODES) % WaveEngine::TOTAL_ARP_MODES;
                    engine.updateSequence(rhythmPatternIdx, arpModeIdx);
                    break;
                case 8: // Delay Time
                    engine.delayTimeMs = std::clamp(engine.delayTimeMs + dir * 10.0f, 50.0f, 500.0f);
                    break;
                case 9: // Delay Feedback
                    engine.delayFeedback = std::clamp(engine.delayFeedback + dir * 0.05f, 0.0f, 0.90f);
                    break;
                case 10: // PLAY / STOP
                    isPlaying = !isPlaying;
                    engine.isPlaying = isPlaying;
                    if (isPlaying) engine.resetClock();
                    break;
                default:
                    break;
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

        if (menuOverlayTimer == 0) {
            menuOverlayTimer = 90;
            return;
        }

        if (currentMenuItem == 10) { // PLAY / STOP
            isPlaying = !isPlaying;
            engine.isPlaying = isPlaying;
            if (isPlaying) engine.resetClock();
        } else {
            isEditing = !isEditing;
        }
    }

    void exitToMainView()
    {
        isEditing = false;
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
