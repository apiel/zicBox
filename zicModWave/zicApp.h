#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "audioEngine.h"

enum PotIndex {
    POT_PITCH = 0,
    POT_WAVE,
    POT_CUTOFF,
    POT_RESONANCE,
    POT_RELEASE,
    POT_ENV_AMT,
    POT_FILT_MORPH,
    POT_CRUSH_FM,
    NUM_POTS
};

class ZicApp {
public:
    WaveEngine engine;

    float potValues[NUM_POTS] = { 0.25f, 0.3f, 0.4f, 0.3f, 0.12f, 0.4f, 0.0f, 0.5f };
    int32_t potOverlayTimer = 0;
    int lastMovedPotIndex = 0;

    int currentMenuItem = 0;
    bool isEditing = false;

    bool isExternalClock = false;
    uint32_t lastMidiClockTimeMs = 0;
    bool isPlaying = true;
    uint32_t autoTriggerTimerMs = 0;

    ZicApp(float sr = 44100.0f)
        : engine(sr)
    {
        syncPotsToEngine();
    }

    void syncPotsToEngine()
    {
        engine.pitch.set(24.0f + potValues[POT_PITCH] * 48.0f);
        engine.waveform.set(potValues[POT_WAVE]);
        engine.cutoff.set(0.02f + potValues[POT_CUTOFF] * 0.96f);
        engine.resonance.set(potValues[POT_RESONANCE] * 0.95f);
        engine.release.set(10.0f + potValues[POT_RELEASE] * 1990.0f);
        engine.envAmt.set(potValues[POT_ENV_AMT]);
        engine.filterMorph.set(potValues[POT_FILT_MORPH]);
        engine.crushFm.set(potValues[POT_CRUSH_FM] * 200.0f - 100.0f);
    }

    void applyPotValue(PotIndex pot, float normVal)
    {
        if (pot < 0 || pot >= NUM_POTS) return;
        normVal = std::clamp(normVal, 0.0f, 1.0f);
        potValues[pot] = normVal;
        lastMovedPotIndex = pot;
        potOverlayTimer = 90; // ~1.5 sec overlay timer at 60 FPS

        switch (pot) {
            case POT_PITCH:
                engine.pitch.set(24.0f + normVal * 48.0f);
                break;
            case POT_WAVE:
                engine.waveform.set(normVal);
                break;
            case POT_CUTOFF:
                engine.cutoff.set(0.02f + normVal * 0.96f);
                break;
            case POT_RESONANCE:
                engine.resonance.set(normVal * 0.95f);
                break;
            case POT_RELEASE:
                engine.release.set(10.0f + normVal * 1990.0f);
                break;
            case POT_ENV_AMT:
                engine.envAmt.set(normVal);
                break;
            case POT_FILT_MORPH:
                engine.filterMorph.set(normVal);
                break;
            case POT_CRUSH_FM:
                engine.crushFm.set(normVal * 200.0f - 100.0f);
                break;
            default:
                break;
        }
    }

    const char* getPotName(PotIndex pot) const
    {
        switch (pot) {
            case POT_PITCH:      return "Pitch";
            case POT_WAVE:       return "Wave";
            case POT_CUTOFF:     return "Cutoff";
            case POT_RESONANCE:  return "Reso";
            case POT_RELEASE:    return "Release";
            case POT_ENV_AMT:    return "Env Amt";
            case POT_FILT_MORPH: return "Filt Morph";
            case POT_CRUSH_FM:   return "Crsh / FM";
            default:             return "";
        }
    }

    void getPotFormattedValue(PotIndex pot, char* buf, size_t bufSize) const
    {
        switch (pot) {
            case POT_PITCH: {
                int noteNum = (int)engine.pitch.value;
                static const char* NOTE_NAMES[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
                int oct = (noteNum / 12) - 1;
                int nameIdx = noteNum % 12;
                snprintf(buf, bufSize, "%s%d (%d)", NOTE_NAMES[nameIdx], oct, noteNum);
                break;
            }
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
            case POT_CUTOFF:
                snprintf(buf, bufSize, "%.0f %%", engine.cutoff.value * 100.0f);
                break;
            case POT_RESONANCE:
                snprintf(buf, bufSize, "%.0f %%", engine.resonance.value * 100.0f);
                break;
            case POT_RELEASE:
                snprintf(buf, bufSize, "%.0f ms", engine.release.value);
                break;
            case POT_ENV_AMT:
                snprintf(buf, bufSize, "%.0f %%", engine.envAmt.value * 100.0f);
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
            case POT_CRUSH_FM:
                if (engine.crushFm.value < 0.0f) {
                    snprintf(buf, bufSize, "Crush %.0f%%", -engine.crushFm.value);
                } else if (engine.crushFm.value > 0.0f) {
                    snprintf(buf, bufSize, "FM %.0f%%", engine.crushFm.value);
                } else {
                    snprintf(buf, bufSize, "Clean");
                }
                break;
            default:
                snprintf(buf, bufSize, "0");
                break;
        }
    }

    static constexpr int NUM_MENU_ITEMS = 7;

    const char* getMenuItemName(int index) const
    {
        switch (index) {
            case 0: return "Mod Type";
            case 1: return "Mod Depth";
            case 2: return "Mod Speed";
            case 3: return "Dly Send";
            case 4: return "Volume";
            case 5: return "BPM";
            case 6: return "PLAY / STOP";
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
                snprintf(buf, bufSize, "%+.0f %%", engine.modDepth.value);
                break;
            case 2:
                snprintf(buf, bufSize, "%.0f %%", engine.modSpeed.value);
                break;
            case 3:
                snprintf(buf, bufSize, "%.0f %%", engine.delaySend.value);
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
                snprintf(buf, bufSize, "%s", isPlaying ? "PLAYING" : "STOPPED");
                break;
            default:
                snprintf(buf, bufSize, "-");
                break;
        }
    }

    void handleEncoderTurn(int dir)
    {
        if (potOverlayTimer > 0) {
            potOverlayTimer = 0; // Dismiss pot overlay on encoder turn
        }

        if (!isEditing) {
            currentMenuItem = (currentMenuItem + dir + NUM_MENU_ITEMS) % NUM_MENU_ITEMS;
        } else {
            switch (currentMenuItem) {
                case 0: { // Mod Type
                    float v = engine.modType.value + dir;
                    v = std::clamp(v, 0.0f, 15.0f);
                    engine.modType.set(v);
                    break;
                }
                case 1: // Mod Depth
                    engine.modDepth.set(std::clamp(engine.modDepth.value + dir * 2.0f, -100.0f, 100.0f));
                    break;
                case 2: // Mod Speed
                    engine.modSpeed.set(std::clamp(engine.modSpeed.value + dir * 2.0f, 0.0f, 100.0f));
                    break;
                case 3: // Delay Send
                    engine.delaySend.set(std::clamp(engine.delaySend.value + dir * 2.0f, 0.0f, 100.0f));
                    break;
                case 4: // Volume
                    engine.masterVol.set(std::clamp(engine.masterVol.value + dir * 2.0f, 0.0f, 100.0f));
                    break;
                case 5: // BPM
                    engine.bpmParam.set(std::clamp(engine.bpmParam.value + dir * 1.0f, 40.0f, 240.0f));
                    break;
                case 6: // PLAY / STOP
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
        if (potOverlayTimer > 0) {
            potOverlayTimer = 0;
            return;
        }

        if (currentMenuItem == 6) { // PLAY / STOP toggle
            isPlaying = !isPlaying;
            engine.isPlaying = isPlaying;
            if (isPlaying) engine.resetClock();
        } else {
            isEditing = !isEditing;
        }
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
        if (byte == 0xF8) { // MIDI Clock pulse
            isExternalClock = true;
            engine.isExternalClock = true;
            lastMidiClockTimeMs = currentMs;
            if (isPlaying) {
                engine.onMidiClockPulse();
            }
        } else if (byte == 0xFA) { // MIDI Start
            isPlaying = true;
            engine.isPlaying = true;
            isExternalClock = true;
            engine.isExternalClock = true;
            lastMidiClockTimeMs = currentMs;
            engine.resetClock();
        } else if (byte == 0xFC) { // MIDI Stop
            isPlaying = false;
            engine.isPlaying = false;
        } else if ((byte & 0xF0) == 0x90) { // Note On
            engine.synth.noteOn(byte, 1.0f);
        }
    }
};
