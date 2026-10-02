#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

#include "audioEngine.h"
#include "audio/engines/SynthAcid303.h"
#include "MasterFX.h"

enum PotIndex {
    POT_1 = 0,
    POT_2,
    POT_3,
    POT_4,
    POT_5,
    POT_6,
    POT_7,
    POT_8,
    NUM_POTS
};

// Aliases for ModWave backward compatibility
constexpr PotIndex POT_WAVE = POT_1;
constexpr PotIndex POT_CRUSH_FM = POT_2;
constexpr PotIndex POT_CUTOFF = POT_3;
constexpr PotIndex POT_RESONANCE = POT_4;
constexpr PotIndex POT_RING_MOD = POT_5;
constexpr PotIndex POT_MOD_DEPTH = POT_6;
constexpr PotIndex POT_MOD_SPEED = POT_7;
constexpr PotIndex POT_DLY_SEND = POT_8;

class ZicApp {
public:
    WaveEngine engineModWave;
    SynthAcid303 engineAcid303;
    MasterFX masterFX;

    static constexpr int TOTAL_ENGINES = 2;
    IEngine* engines[TOTAL_ENGINES] = { &engineModWave, &engineAcid303 };
    int activeEngineIdx = 0; // 0: ModWave, 1: Acid 303

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
        : engineModWave(sr)
        , engineAcid303(sr)
    {
        syncPotsToEngine();
        updateSequence(rhythmPatternIdx, arpModeIdx);
    }

    ~ZicApp() = default;

    IEngine* getActiveEngine() const
    {
        return engines[activeEngineIdx];
    }

    bool hasCustomUI() const
    {
        return (activeEngineIdx == 0); // ModWave engine has custom 3D visualizer
    }

    void updateSequence(int rhythmIdx, int arpIdx)
    {
        rhythmPatternIdx = std::clamp(rhythmIdx, 0, WaveEngine::TOTAL_RHYTHM_PATTERNS - 1);
        arpModeIdx = std::clamp(arpIdx, 0, WaveEngine::TOTAL_ARP_MODES - 1);
        engineModWave.updateSequence(rhythmPatternIdx, arpModeIdx);
    }

    void syncPotsToEngine()
    {
        if (activeEngineIdx == 0) { // ModWave
            engineModWave.waveform.set(potValues[POT_1]);
            engineModWave.crushFm.set(potValues[POT_2] * 100.0f);
            engineModWave.cutoff.set(0.02f + potValues[POT_3] * 0.96f);
            engineModWave.resonance.set(potValues[POT_4] * 0.95f);
            engineModWave.ringMod.set(potValues[POT_5]);
            engineModWave.modDepth.set(potValues[POT_6] * 200.0f - 100.0f);
            engineModWave.modSpeed.set(potValues[POT_7] * 100.0f);
            engineModWave.delaySend.set(potValues[POT_8] * 100.0f);
        } else if (activeEngineIdx == 1) { // Acid 303
            engineAcid303.cutoff.set(0.02f + potValues[POT_1] * 0.96f);
            engineAcid303.resonance.set(potValues[POT_2] * 0.95f);
            engineAcid303.envAmt.set(potValues[POT_3]);
            engineAcid303.decayMs.set(50.0f + potValues[POT_4] * 1950.0f);
            engineAcid303.accent.set(potValues[POT_5]);
            engineAcid303.drive.set(potValues[POT_6]);
            engineAcid303.waveMorph.set(potValues[POT_7]);
            engineAcid303.delaySend.set(potValues[POT_8] * 100.0f);
        }
    }

    void applyPotValue(PotIndex pot, float normVal)
    {
        if (pot < 0 || pot >= NUM_POTS) return;
        normVal = std::clamp(normVal, 0.0f, 1.0f);
        potValues[pot] = normVal;
        lastMovedPotIndex = pot;
        potOverlayTimer = 90; // ~1.5 sec toast overlay timer at 60 FPS

        if (activeEngineIdx == 0) { // ModWave
            switch (pot) {
                case POT_1: engineModWave.waveform.set(normVal); break;
                case POT_2: engineModWave.crushFm.set(normVal * 100.0f); break;
                case POT_3: engineModWave.cutoff.set(0.02f + normVal * 0.96f); break;
                case POT_4: engineModWave.resonance.set(normVal * 0.95f); break;
                case POT_5: engineModWave.ringMod.set(normVal); break;
                case POT_6: engineModWave.modDepth.set(normVal * 200.0f - 100.0f); break;
                case POT_7: engineModWave.modSpeed.set(normVal * 100.0f); break;
                case POT_8: engineModWave.delaySend.set(normVal * 100.0f); break;
                default: break;
            }
        } else if (activeEngineIdx == 1) { // Acid 303
            switch (pot) {
                case POT_1: engineAcid303.cutoff.set(0.02f + normVal * 0.96f); break;
                case POT_2: engineAcid303.resonance.set(normVal * 0.95f); break;
                case POT_3: engineAcid303.envAmt.set(normVal); break;
                case POT_4: engineAcid303.decayMs.set(50.0f + normVal * 1950.0f); break;
                case POT_5: engineAcid303.accent.set(normVal); break;
                case POT_6: engineAcid303.drive.set(normVal); break;
                case POT_7: engineAcid303.waveMorph.set(normVal); break;
                case POT_8: engineAcid303.delaySend.set(normVal * 100.0f); break;
                default: break;
            }
        }
    }

    const char* getPotName(PotIndex pot) const
    {
        if (activeEngineIdx == 0) { // ModWave
            switch (pot) {
                case POT_1: return "Wave";
                case POT_2: {
                    float noiseFade = std::clamp((engineModWave.waveform.value - 0.50f) / 0.30f, 0.0f, 1.0f);
                    return (noiseFade < 0.2f) ? "FM Depth" : ((noiseFade > 0.8f) ? "Bitcrush" : "FM + Crush");
                }
                case POT_3: return "Cutoff";
                case POT_4: return "Reso";
                case POT_5: return "Ring Mod";
                case POT_6: return "Mod Depth";
                case POT_7: return "Mod Speed";
                case POT_8: return "Dly Send";
                default: return "";
            }
        } else { // Acid 303
            switch (pot) {
                case POT_1: return "Cutoff";
                case POT_2: return "Reso";
                case POT_3: return "Env Amt";
                case POT_4: return "Decay";
                case POT_5: return "Accent";
                case POT_6: return "Drive";
                case POT_7: return "Wave Morph";
                case POT_8: return "Dly Send";
                default: return "";
            }
        }
    }

    void getPotFormattedValue(PotIndex pot, char* buf, size_t bufSize) const
    {
        if (activeEngineIdx == 0) { // ModWave
            switch (pot) {
                case POT_1: {
                    float wf = engineModWave.waveform.value;
                    if (wf < 0.333f) snprintf(buf, bufSize, "Tri->Saw (%.0f%%)", wf * 300.0f);
                    else if (wf < 0.666f) snprintf(buf, bufSize, "Saw->Sq (%.0f%%)", (wf - 0.333f) * 300.0f);
                    else snprintf(buf, bufSize, "Sq->Noise (%.0f%%)", (wf - 0.666f) * 300.0f);
                    break;
                }
                case POT_2: {
                    float val = engineModWave.crushFm.value;
                    if (val <= 0.5f) snprintf(buf, bufSize, "Clean");
                    else snprintf(buf, bufSize, "ModFX %.0f%%", val);
                    break;
                }
                case POT_3: snprintf(buf, bufSize, "%.0f %%", engineModWave.cutoff.value * 100.0f); break;
                case POT_4: snprintf(buf, bufSize, "%.0f %%", engineModWave.resonance.value * 100.0f); break;
                case POT_5: snprintf(buf, bufSize, "%.0f %%", engineModWave.ringMod.value * 100.0f); break;
                case POT_6: snprintf(buf, bufSize, "%+.0f %%", engineModWave.modDepth.value); break;
                case POT_7: snprintf(buf, bufSize, "%.0f %%", engineModWave.modSpeed.value); break;
                case POT_8: snprintf(buf, bufSize, "%.0f %%", engineModWave.delaySend.value); break;
                default: snprintf(buf, bufSize, "0"); break;
            }
        } else { // Acid 303
            switch (pot) {
                case POT_1: snprintf(buf, bufSize, "%.0f %%", engineAcid303.cutoff.value * 100.0f); break;
                case POT_2: snprintf(buf, bufSize, "%.0f %%", engineAcid303.resonance.value * 100.0f); break;
                case POT_3: snprintf(buf, bufSize, "%.0f %%", engineAcid303.envAmt.value * 100.0f); break;
                case POT_4: snprintf(buf, bufSize, "%.0f ms", engineAcid303.decayMs.value); break;
                case POT_5: snprintf(buf, bufSize, "%.0f %%", engineAcid303.accent.value * 100.0f); break;
                case POT_6: snprintf(buf, bufSize, "%.0f %%", engineAcid303.drive.value * 100.0f); break;
                case POT_7: {
                    float w = engineAcid303.waveMorph.value;
                    snprintf(buf, bufSize, "Saw->Sq (%.0f%%)", w * 100.0f);
                    break;
                }
                case POT_8: snprintf(buf, bufSize, "%.0f %%", engineAcid303.delaySend.value); break;
                default: snprintf(buf, bufSize, "0"); break;
            }
        }
    }

    int getNumMenuItems() const
    {
        return (activeEngineIdx == 0) ? 13 : 10;
    }

    const char* getMenuItemName(int index) const
    {
        if (activeEngineIdx == 0) { // ModWave
            switch (index) {
                case 0: return "Engine";
                case 1: return "Mod Type";
                case 2: return "FM Ratio";
                case 3: return "Pitch";
                case 4: return "Env Amt";
                case 5: return "Release";
                case 6: return "Volume";
                case 7: return "BPM";
                case 8: return "Rhythm";
                case 9: return "Arp Mode";
                case 10: return "Delay Time";
                case 11: return "Delay FB";
                case 12: return "PLAY / STOP";
                default: return "";
            }
        } else { // Acid 303
            switch (index) {
                case 0: return "Engine";
                case 1: return "Pitch";
                case 2: return "Sub Level";
                case 3: return "Volume";
                case 4: return "BPM";
                case 5: return "Rhythm";
                case 6: return "Arp Mode";
                case 7: return "Delay Time";
                case 8: return "Delay FB";
                case 9: return "PLAY / STOP";
                default: return "";
            }
        }
    }

    void getMenuItemFormattedValue(int index, char* buf, size_t bufSize) const
    {
        if (index == 0) {
            snprintf(buf, bufSize, "%s (%d/%d)", engines[activeEngineIdx]->getName(), activeEngineIdx + 1, TOTAL_ENGINES);
            return;
        }

        if (activeEngineIdx == 0) { // ModWave
            switch (index) {
                case 1: snprintf(buf, bufSize, "%s", engineModWave.modTypeNameDisplay); break;
                case 2: snprintf(buf, bufSize, "%.1f x", engineModWave.fmRatioParam.value); break;
                case 3: {
                    int noteNum = (int)engineModWave.pitch.value;
                    static const char* NOTE_NAMES[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
                    snprintf(buf, bufSize, "%s%d (%d)", NOTE_NAMES[noteNum % 12], (noteNum / 12) - 1, noteNum);
                    break;
                }
                case 4: snprintf(buf, bufSize, "%.0f %%", engineModWave.envAmt.value * 100.0f); break;
                case 5: snprintf(buf, bufSize, "%.0f ms", engineModWave.release.value); break;
                case 6: snprintf(buf, bufSize, "%.0f %%", masterFX.masterVol); break;
                case 7:
                    if (isExternalClock) snprintf(buf, bufSize, "MIDI SYNC");
                    else snprintf(buf, bufSize, "%.0f BPM", engineModWave.bpmParam.value);
                    break;
                case 8: snprintf(buf, bufSize, "%s", WaveEngine::RHYTHM_NAMES[rhythmPatternIdx]); break;
                case 9: snprintf(buf, bufSize, "%s", WaveEngine::ARP_NAMES[arpModeIdx]); break;
                case 10: snprintf(buf, bufSize, "%.0f ms", masterFX.delayTimeMs); break;
                case 11: snprintf(buf, bufSize, "%.0f %%", masterFX.delayFeedback * 100.0f); break;
                case 12: snprintf(buf, bufSize, "%s", isPlaying ? "PLAYING" : "STOPPED"); break;
                default: snprintf(buf, bufSize, "-"); break;
            }
        } else { // Acid 303
            switch (index) {
                case 1: {
                    int noteNum = (int)engineAcid303.pitch.value;
                    static const char* NOTE_NAMES[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
                    snprintf(buf, bufSize, "%s%d (%d)", NOTE_NAMES[noteNum % 12], (noteNum / 12) - 1, noteNum);
                    break;
                }
                case 2: snprintf(buf, bufSize, "%.0f %%", engineAcid303.subLevel.value * 100.0f); break;
                case 3: snprintf(buf, bufSize, "%.0f %%", masterFX.masterVol); break;
                case 4:
                    if (isExternalClock) snprintf(buf, bufSize, "MIDI SYNC");
                    else snprintf(buf, bufSize, "%.0f BPM", engineModWave.bpmParam.value);
                    break;
                case 5: snprintf(buf, bufSize, "%s", WaveEngine::RHYTHM_NAMES[rhythmPatternIdx]); break;
                case 6: snprintf(buf, bufSize, "%s", WaveEngine::ARP_NAMES[arpModeIdx]); break;
                case 7: snprintf(buf, bufSize, "%.0f ms", masterFX.delayTimeMs); break;
                case 8: snprintf(buf, bufSize, "%.0f %%", masterFX.delayFeedback * 100.0f); break;
                case 9: snprintf(buf, bufSize, "%s", isPlaying ? "PLAYING" : "STOPPED"); break;
                default: snprintf(buf, bufSize, "-"); break;
            }
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

        int numItems = getNumMenuItems();

        if (!isEditing) {
            currentMenuItem = (currentMenuItem + dir + numItems) % numItems;
        } else {
            if (currentMenuItem == 0) { // Engine Selection
                activeEngineIdx = (activeEngineIdx + dir + TOTAL_ENGINES) % TOTAL_ENGINES;
                syncPotsToEngine();
                currentMenuItem = 0;
                return;
            }

            if (activeEngineIdx == 0) { // ModWave
                switch (currentMenuItem) {
                    case 1: { float v = std::clamp(engineModWave.modType.value + dir, 0.0f, 15.0f); engineModWave.modType.set(v); break; }
                    case 2: { float v = std::clamp(engineModWave.fmRatioParam.value + dir * 0.5f, 0.5f, 8.0f); engineModWave.fmRatioParam.set(v); break; }
                    case 3: { float v = std::clamp(engineModWave.pitch.value + dir, 24.0f, 72.0f); engineModWave.pitch.set(v); break; }
                    case 4: { float v = std::clamp(engineModWave.envAmt.value + dir * 0.05f, 0.0f, 1.0f); engineModWave.envAmt.set(v); break; }
                    case 5: { float v = std::clamp(engineModWave.release.value + dir * 25.0f, 10.0f, 2000.0f); engineModWave.release.set(v); break; }
                    case 6: masterFX.masterVol = std::clamp(masterFX.masterVol + dir * 2.0f, 0.0f, 100.0f); break;
                    case 7: engineModWave.bpmParam.set(std::clamp(engineModWave.bpmParam.value + dir * 1.0f, 40.0f, 240.0f)); break;
                    case 8:
                        rhythmPatternIdx = (rhythmPatternIdx + dir + WaveEngine::TOTAL_RHYTHM_PATTERNS) % WaveEngine::TOTAL_RHYTHM_PATTERNS;
                        updateSequence(rhythmPatternIdx, arpModeIdx);
                        break;
                    case 9:
                        arpModeIdx = (arpModeIdx + dir + WaveEngine::TOTAL_ARP_MODES) % WaveEngine::TOTAL_ARP_MODES;
                        updateSequence(rhythmPatternIdx, arpModeIdx);
                        break;
                    case 10: masterFX.delayTimeMs = std::clamp(masterFX.delayTimeMs + dir * 10.0f, 50.0f, 500.0f); break;
                    case 11: masterFX.delayFeedback = std::clamp(masterFX.delayFeedback + dir * 0.05f, 0.0f, 0.90f); break;
                    case 12:
                        isPlaying = !isPlaying;
                        engineModWave.isPlaying = isPlaying;
                        if (isPlaying) engineModWave.resetClock();
                        break;
                    default: break;
                }
            } else { // Acid 303
                switch (currentMenuItem) {
                    case 1: { float v = std::clamp(engineAcid303.pitch.value + dir, 24.0f, 72.0f); engineAcid303.pitch.set(v); break; }
                    case 2: { float v = std::clamp(engineAcid303.subLevel.value + dir * 0.05f, 0.0f, 1.0f); engineAcid303.subLevel.set(v); break; }
                    case 3: masterFX.masterVol = std::clamp(masterFX.masterVol + dir * 2.0f, 0.0f, 100.0f); break;
                    case 4: engineModWave.bpmParam.set(std::clamp(engineModWave.bpmParam.value + dir * 1.0f, 40.0f, 240.0f)); break;
                    case 5:
                        rhythmPatternIdx = (rhythmPatternIdx + dir + WaveEngine::TOTAL_RHYTHM_PATTERNS) % WaveEngine::TOTAL_RHYTHM_PATTERNS;
                        updateSequence(rhythmPatternIdx, arpModeIdx);
                        break;
                    case 6:
                        arpModeIdx = (arpModeIdx + dir + WaveEngine::TOTAL_ARP_MODES) % WaveEngine::TOTAL_ARP_MODES;
                        updateSequence(rhythmPatternIdx, arpModeIdx);
                        break;
                    case 7: masterFX.delayTimeMs = std::clamp(masterFX.delayTimeMs + dir * 10.0f, 50.0f, 500.0f); break;
                    case 8: masterFX.delayFeedback = std::clamp(masterFX.delayFeedback + dir * 0.05f, 0.0f, 0.90f); break;
                    case 9:
                        isPlaying = !isPlaying;
                        engineModWave.isPlaying = isPlaying;
                        if (isPlaying) engineModWave.resetClock();
                        break;
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

        if (menuOverlayTimer == 0) {
            menuOverlayTimer = 90;
            return;
        }

        int playStopIdx = (activeEngineIdx == 0) ? 12 : 9;
        if (currentMenuItem == playStopIdx) { // PLAY / STOP
            isPlaying = !isPlaying;
            engineModWave.isPlaying = isPlaying;
            if (isPlaying) engineModWave.resetClock();
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
            engineModWave.isExternalClock = false;
        }
    }

    void handleMidiByte(uint8_t byte, uint32_t currentMs)
    {
        if (byte == 0xF8) {
            isExternalClock = true;
            engineModWave.isExternalClock = true;
            lastMidiClockTimeMs = currentMs;
            if (isPlaying) {
                engineModWave.onMidiClockPulse();
            }
        } else if (byte == 0xFA) {
            isPlaying = true;
            engineModWave.isPlaying = true;
            isExternalClock = true;
            engineModWave.isExternalClock = true;
            lastMidiClockTimeMs = currentMs;
            engineModWave.resetClock();
        } else if (byte == 0xFC) {
            isPlaying = false;
            engineModWave.isPlaying = false;
        } else if ((byte & 0xF0) == 0x90) {
            engines[activeEngineIdx]->noteOn(byte, 1.0f);
        }
    }
};
