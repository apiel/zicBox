#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "audioEngine.h"

enum PotIndex {
    POT_DENSITY = 0,
    POT_LOOP_START,
    POT_LOOP_LEN,
    POT_GRAIN_DETUNE,
    POT_GRAIN_DELAY,
    POT_RAND_DETUNE,
    POT_RAND_DELAY,
    POT_CUTOFF,
    NUM_POTS
};

class ZicApp {
public:
    GranularEngine engine;

    float potValues[NUM_POTS] = { 0.25f, 0.0f, 0.25f, 0.0f, 0.16f, 0.0f, 0.0f, 0.5f };
    int32_t potOverlayTimer = 0;
    int lastMovedPotIndex = 0;

    int currentMenuItem = 0;
    bool isEditing = false;

    bool isExternalClock = false;
    uint32_t lastMidiClockTimeMs = 0;
    bool isPlaying = true;

    ZicApp(float sr = 44100.0f)
        : engine(sr)
    {
        syncPotsToEngine();
    }

    void syncPotsToEngine()
    {
        engine.density.set(1.0f + potValues[POT_DENSITY] * 15.0f);
        engine.loopStart.set(potValues[POT_LOOP_START] * 100.0f);
        engine.loopLength.set(potValues[POT_LOOP_LEN] * 4000.0f);
        engine.grainDetune.set(potValues[POT_GRAIN_DETUNE] * 12.0f);
        engine.grainDelay.set(5.0f + potValues[POT_GRAIN_DELAY] * 495.0f);
        engine.pitchRnd.set(potValues[POT_RAND_DETUNE] * 100.0f);
        engine.delayRnd.set(potValues[POT_RAND_DELAY] * 100.0f);
        engine.cutoff.set(potValues[POT_CUTOFF] * 200.0f - 100.0f);
    }

    void applyPotValue(PotIndex pot, float normVal)
    {
        if (pot < 0 || pot >= NUM_POTS) return;
        normVal = std::clamp(normVal, 0.0f, 1.0f);
        potValues[pot] = normVal;
        lastMovedPotIndex = pot;
        potOverlayTimer = 90; // ~1.5 sec overlay timer at 60 FPS

        switch (pot) {
            case POT_DENSITY:
                engine.density.set(1.0f + normVal * 15.0f);
                break;
            case POT_LOOP_START:
                engine.loopStart.set(normVal * 100.0f);
                break;
            case POT_LOOP_LEN:
                engine.loopLength.set(normVal * 4000.0f);
                break;
            case POT_GRAIN_DETUNE:
                engine.grainDetune.set(normVal * 12.0f);
                break;
            case POT_GRAIN_DELAY:
                engine.grainDelay.set(5.0f + normVal * 495.0f);
                break;
            case POT_RAND_DETUNE:
                engine.pitchRnd.set(normVal * 100.0f);
                break;
            case POT_RAND_DELAY:
                engine.delayRnd.set(normVal * 100.0f);
                break;
            case POT_CUTOFF:
                engine.cutoff.set(normVal * 200.0f - 100.0f);
                break;
            default:
                break;
        }
    }

    const char* getPotName(PotIndex pot) const
    {
        switch (pot) {
            case POT_DENSITY:      return "Density";
            case POT_LOOP_START:   return "Loop Start";
            case POT_LOOP_LEN:     return "Loop Len";
            case POT_GRAIN_DETUNE: return "Grain Detune";
            case POT_GRAIN_DELAY:  return "Grain Delay";
            case POT_RAND_DETUNE:  return "Rand Detune";
            case POT_RAND_DELAY:   return "Rand Delay";
            case POT_CUTOFF:       return "Cutoff";
            default:               return "";
        }
    }

    void getPotFormattedValue(PotIndex pot, char* buf, size_t bufSize) const
    {
        switch (pot) {
            case POT_DENSITY:
                snprintf(buf, bufSize, "%d Grains", (int)engine.density.value);
                break;
            case POT_LOOP_START:
                snprintf(buf, bufSize, "%.1f %%", engine.loopStart.value);
                break;
            case POT_LOOP_LEN:
                snprintf(buf, bufSize, "%.0f ms", engine.loopLength.value);
                break;
            case POT_GRAIN_DETUNE:
                snprintf(buf, bufSize, "%.1f st", engine.grainDetune.value);
                break;
            case POT_GRAIN_DELAY:
                snprintf(buf, bufSize, "%.0f ms", engine.grainDelay.value);
                break;
            case POT_RAND_DETUNE:
                snprintf(buf, bufSize, "%.0f %%", engine.pitchRnd.value);
                break;
            case POT_RAND_DELAY:
                snprintf(buf, bufSize, "%.0f %%", engine.delayRnd.value);
                break;
            case POT_CUTOFF:
                snprintf(buf, bufSize, "%.0f %%", engine.cutoff.value);
                break;
            default:
                snprintf(buf, bufSize, "0");
                break;
        }
    }

    static constexpr int NUM_MENU_ITEMS = 12;

    const char* getMenuItemName(int index) const
    {
        switch (index) {
            case 0:  return "File";
            case 1:  return "Start";
            case 2:  return "End";
            case 3:  return "Attack";
            case 4:  return "Release";
            case 5:  return "Detune Mode";
            case 6:  return "Direction";
            case 7:  return "Resonance";
            case 8:  return "Transpose";
            case 9:  return "Volume";
            case 10: return "BPM";
            case 11: return "PLAY / STOP";
            default: return "";
        }
    }

    void getMenuItemFormattedValue(int index, char* buf, size_t bufSize) const
    {
        switch (index) {
            case 0:
                snprintf(buf, bufSize, "%s", engine.fileNameDisplay);
                break;
            case 1:
                snprintf(buf, bufSize, "%.1f %%", engine.sampleStart.value);
                break;
            case 2:
                snprintf(buf, bufSize, "%.1f %%", engine.sampleEnd.value);
                break;
            case 3:
                snprintf(buf, bufSize, "%.1f %%", engine.envAttack.value);
                break;
            case 4:
                snprintf(buf, bufSize, "%.1f %%", engine.envRelease.value);
                break;
            case 5:
                snprintf(buf, bufSize, "%s", engine.detunModeName);
                break;
            case 6:
                snprintf(buf, bufSize, "%s", engine.directionName);
                break;
            case 7:
                snprintf(buf, bufSize, "%.0f %%", engine.resonance.value);
                break;
            case 8:
                snprintf(buf, bufSize, "%+.0f st", engine.transpose.value);
                break;
            case 9:
                snprintf(buf, bufSize, "%.0f %%", engine.masterVol.value);
                break;
            case 10:
                if (isExternalClock) {
                    snprintf(buf, bufSize, "MIDI SYNC");
                } else {
                    snprintf(buf, bufSize, "%.0f BPM", engine.bpmParam.value);
                }
                break;
            case 11:
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
                case 0: { // File
                    float v = engine.sampleSelect.value + dir;
                    v = std::clamp(v, 0.0f, engine.sampleSelect.max);
                    engine.sampleSelect.set(v);
                    break;
                }
                case 1: // Start
                    engine.sampleStart.set(std::clamp(engine.sampleStart.value + dir * 0.5f, 0.0f, 100.0f));
                    break;
                case 2: // End
                    engine.sampleEnd.set(std::clamp(engine.sampleEnd.value + dir * 0.5f, 0.0f, 100.0f));
                    break;
                case 3: // Attack
                    engine.envAttack.set(std::clamp(engine.envAttack.value + dir * 0.5f, 0.0f, 100.0f));
                    break;
                case 4: // Release
                    engine.envRelease.set(std::clamp(engine.envRelease.value + dir * 0.5f, 0.0f, 100.0f));
                    break;
                case 5: { // Detune Mode
                    float v = engine.detuneMode.value + dir;
                    v = std::clamp(v, 1.0f, 3.0f);
                    engine.detuneMode.set(v);
                    break;
                }
                case 6: { // Direction
                    float v = engine.direction.value + dir;
                    v = std::clamp(v, 1.0f, 3.0f);
                    engine.direction.set(v);
                    break;
                }
                case 7: // Resonance
                    engine.resonance.set(std::clamp(engine.resonance.value + dir * 1.0f, 0.0f, 100.0f));
                    break;
                case 8: // Transpose
                    engine.transpose.set(std::clamp(engine.transpose.value + dir * 1.0f, -24.0f, 24.0f));
                    break;
                case 9: // Master Volume
                    engine.masterVol.set(std::clamp(engine.masterVol.value + dir * 2.0f, 0.0f, 100.0f));
                    break;
                case 10: // BPM
                    engine.bpmParam.set(std::clamp(engine.bpmParam.value + dir * 1.0f, 40.0f, 240.0f));
                    break;
                case 11: // PLAY / STOP
                    isPlaying = !isPlaying;
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

        if (currentMenuItem == 11) { // PLAY / STOP toggle
            isPlaying = !isPlaying;
        } else {
            isEditing = !isEditing;
        }
    }

    void updateMidiClockTimeout(uint32_t currentMs)
    {
        if (isExternalClock && (currentMs - lastMidiClockTimeMs > 500)) {
            isExternalClock = false;
        }
    }

    void handleMidiByte(uint8_t byte, uint32_t currentMs)
    {
        if (byte == 0xF8) { // MIDI Clock pulse
            isExternalClock = true;
            lastMidiClockTimeMs = currentMs;
        } else if (byte == 0xFA) { // MIDI Start
            isPlaying = true;
            isExternalClock = true;
            lastMidiClockTimeMs = currentMs;
        } else if (byte == 0xFC) { // MIDI Stop
            isPlaying = false;
        }
    }
};
