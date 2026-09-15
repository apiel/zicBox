#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "SampleTrack.h"
#include "sequenceBrain.h"

#ifdef ARDUINO
#include <Arduino.h>
#include <Preferences.h>
inline uint32_t getSystemMillis() { return millis(); }
#else
#include <fstream>
inline uint32_t getSystemMillis() {
    using namespace std::chrono;
    return (uint32_t)duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
#endif

enum ViewMode {
    VIEW_OVERVIEW = 0,
    VIEW_STEP_EDIT = 1,
    VIEW_SOUND_EDIT = 2,
    VIEW_CLIPS = 3,
    VIEW_PROJECTS = 4,
    VIEW_GLOBAL = 5,
    NUM_VIEWS = 6
};

static const uint8_t PROBABILITY_PRESETS[16] = {
    0,  10, 20, 25,
    33, 40, 50, 60,
    66, 70, 75, 80,
    85, 90, 95, 100
};

struct ProjectSlot {
    bool isOccupied = false;
    float bpm = 125.0f;
    float masterVolume = 1.0f;
    int selectedTrack = 0;
    DrumTrack tracks[SequenceBrain::NUM_TRACKS];
    SampleTrack sampleTracks[SequenceBrain::NUM_TRACKS];
};

enum ProjectCopyState {
    COPY_IDLE = 0,
    COPY_WAIT_TARGET = 1,
    COPY_CONFIRM_OVERWRITE = 2
};



struct PersistedStep {
    uint8_t active;
    uint8_t note;
    uint8_t velocity;
    uint8_t probability;
};

struct PersistedClip {
    PersistedStep steps[16];
    uint8_t sampleIdx;
    int8_t pitch;
    uint8_t volume;
    uint8_t isCreated;
};

struct PersistedTrack {
    uint8_t muted;
    uint8_t activeClip;
    PersistedClip clips[8];
    PersistedStep steps[16];
    uint8_t sampleIdx;
    int8_t pitch;
    uint8_t volume;
};

struct PersistedProjectSlot {
    uint8_t isOccupied;
    uint16_t bpm;
    uint8_t masterVolume;
    uint8_t selectedTrack;
    PersistedTrack tracks[8];
};

class ZicApp {
public:
    float sampleRate = 44100.0f;

    // 8 Sample Playback Tracks
    SampleTrack sampleTracks[8];

    // Sequencer brain
    SequenceBrain brain;

    // Application State
    ViewMode currentView = VIEW_OVERVIEW;
    float masterVolume = 1.0f;
    bool autoTriggerOnSelect = true;
    uint8_t padBrightness = 0; // 0: 25%, 1: 50%, 2: 75%, 3: 100%
    bool globalMenuMode = false;

    uint8_t getNeoBrightnessValue() const
    {
        switch (padBrightness) {
            case 0: return 32;   // 25%
            case 1: return 80;   // 50%
            case 2: return 160;  // 75%
            case 3: return 255;  // 100%
            default: return 32;
        }
    }

    void cycleBrightness()
    {
        padBrightness = (padBrightness + 1) % 4;
        markDirty();
    }

    // Step Probability Sub-Menu State
    bool showProbSubMenu = false;
    uint8_t probEditingStep = 0;
    uint32_t padPressTime[16] = { 0 };
    bool padIsDown[16] = { false };
    bool padLongPressHandled[16] = { false };

    // Project Persistence & Copy State
    ProjectSlot projects[16];
    uint8_t currentProject = 0;
    ProjectCopyState copyState = COPY_IDLE;
    int copySourcePad = -1;
    int copyTargetPad = -1;

    // Persistence Auto-Save state
    bool isDirty = false;
    uint32_t lastChangeTime = 0;

    // External MIDI Clock Sync State & Auto-Fallback
    bool isExternalClock = false;
    uint32_t lastMidiClockMs = 0;
    uint8_t midiTickCounter = 0;
    volatile int pendingStepTriggers = 0;

    ZicApp(float sampleRate = 44100.0f)
        : sampleRate(sampleRate)
        , brain(sampleRate)
    {
        // Initialize 8 sample tracks with default preset samples
        sampleTracks[0].init(0, "Track 1", 0);
        sampleTracks[1].init(1, "Track 2", 1);
        sampleTracks[2].init(2, "Track 3", 2);
        sampleTracks[3].init(3, "Track 4", 3);
        sampleTracks[4].init(4, "Track 5", 4);
        sampleTracks[5].init(5, "Track 6", 5);
        sampleTracks[6].init(6, "Track 7", 6);
        sampleTracks[7].init(7, "Track 8", 7);

        // Initialize default Project 0
        saveCurrentProjectSlot(0);

        // Load persisted state if available
        loadAllFromStorage();
    }

    void markDirty()
    {
        isDirty = true;
        lastChangeTime = getSystemMillis();
    }

    void packProjectSlot(const ProjectSlot& src, PersistedProjectSlot& dst)
    {
        dst.isOccupied = src.isOccupied ? 1 : 0;
        dst.bpm = (uint16_t)std::round(src.bpm);
        dst.masterVolume = (uint8_t)std::clamp((int)(src.masterVolume * 100.0f), 0, 255);
        dst.selectedTrack = (uint8_t)src.selectedTrack;

        for (int t = 0; t < SequenceBrain::NUM_TRACKS; ++t) {
            dst.tracks[t].muted = src.tracks[t].muted ? 1 : 0;
            dst.tracks[t].activeClip = src.tracks[t].activeClip;
            dst.tracks[t].sampleIdx = src.sampleTracks[t].sampleIdx;
            dst.tracks[t].pitch = (int8_t)std::round(src.sampleTracks[t].pitch);
            dst.tracks[t].volume = (uint8_t)std::clamp((int)(src.sampleTracks[t].volume * 100.0f), 0, 255);

            for (int s = 0; s < SequenceBrain::NUM_STEPS; ++s) {
                dst.tracks[t].steps[s].active = src.tracks[t].steps[s].active ? 1 : 0;
                dst.tracks[t].steps[s].note = src.tracks[t].steps[s].note;
                dst.tracks[t].steps[s].velocity = (uint8_t)std::clamp((int)(src.tracks[t].steps[s].velocity * 255.0f), 0, 255);
                dst.tracks[t].steps[s].probability = src.tracks[t].steps[s].probability;
            }

            for (int c = 0; c < 8; ++c) {
                dst.tracks[t].clips[c].isCreated = src.tracks[t].clips[c].isCreated ? 1 : 0;
                dst.tracks[t].clips[c].sampleIdx = src.tracks[t].clips[c].sampleIdx;
                dst.tracks[t].clips[c].pitch = (int8_t)std::round(src.tracks[t].clips[c].pitch);
                dst.tracks[t].clips[c].volume = (uint8_t)std::clamp((int)(src.tracks[t].clips[c].volume * 100.0f), 0, 255);

                for (int s = 0; s < SequenceBrain::NUM_STEPS; ++s) {
                    dst.tracks[t].clips[c].steps[s].active = src.tracks[t].clips[c].steps[s].active ? 1 : 0;
                    dst.tracks[t].clips[c].steps[s].note = src.tracks[t].clips[c].steps[s].note;
                    dst.tracks[t].clips[c].steps[s].velocity = (uint8_t)std::clamp((int)(src.tracks[t].clips[c].steps[s].velocity * 255.0f), 0, 255);
                    dst.tracks[t].clips[c].steps[s].probability = src.tracks[t].clips[c].steps[s].probability;
                }
            }
        }
    }

    void unpackProjectSlot(const PersistedProjectSlot& src, ProjectSlot& dst)
    {
        dst.isOccupied = (src.isOccupied != 0);
        dst.bpm = (float)src.bpm;
        dst.masterVolume = (float)src.masterVolume / 100.0f;
        dst.selectedTrack = src.selectedTrack % SequenceBrain::NUM_TRACKS;

        for (int t = 0; t < SequenceBrain::NUM_TRACKS; ++t) {
            dst.tracks[t].name = brain.tracks[t].name;
            dst.tracks[t].muted = (src.tracks[t].muted != 0);
            dst.tracks[t].activeClip = src.tracks[t].activeClip % 8;

            dst.sampleTracks[t].init(t, brain.tracks[t].name, src.tracks[t].sampleIdx);
            dst.sampleTracks[t].pitch = (float)src.tracks[t].pitch;
            dst.sampleTracks[t].updateSpeed();
            dst.sampleTracks[t].volume = (float)src.tracks[t].volume / 100.0f;

            for (int s = 0; s < SequenceBrain::NUM_STEPS; ++s) {
                dst.tracks[t].steps[s].active = (src.tracks[t].steps[s].active != 0);
                dst.tracks[t].steps[s].note = src.tracks[t].steps[s].note;
                dst.tracks[t].steps[s].velocity = (float)src.tracks[t].steps[s].velocity / 255.0f;
                dst.tracks[t].steps[s].probability = src.tracks[t].steps[s].probability;
            }

            for (int c = 0; c < 8; ++c) {
                dst.tracks[t].clips[c].isCreated = (src.tracks[t].clips[c].isCreated != 0);
                dst.tracks[t].clips[c].sampleIdx = src.tracks[t].clips[c].sampleIdx;
                dst.tracks[t].clips[c].pitch = (float)src.tracks[t].clips[c].pitch;
                dst.tracks[t].clips[c].volume = (float)src.tracks[t].clips[c].volume / 100.0f;

                for (int s = 0; s < SequenceBrain::NUM_STEPS; ++s) {
                    dst.tracks[t].clips[c].steps[s].active = (src.tracks[t].clips[c].steps[s].active != 0);
                    dst.tracks[t].clips[c].steps[s].note = src.tracks[t].clips[c].steps[s].note;
                    dst.tracks[t].clips[c].steps[s].velocity = (float)src.tracks[t].clips[c].steps[s].velocity / 255.0f;
                    dst.tracks[t].clips[c].steps[s].probability = src.tracks[t].clips[c].steps[s].probability;
                }
            }
        }
    }

    void saveProjectSlotToStorage(uint8_t slotIdx)
    {
        if (slotIdx >= 16) return;
        saveCurrentProjectSlot(slotIdx);

        PersistedProjectSlot pData;
        packProjectSlot(projects[slotIdx], pData);

        #ifdef ARDUINO
        Preferences prefs;
        if (prefs.begin("zicApp", false)) {
            char key[16];
            snprintf(key, sizeof(key), "p_%d", slotIdx);
            prefs.putBytes(key, &pData, sizeof(pData));
            prefs.putUChar("curr_p", currentProject);
            prefs.putUChar("pad_bright", padBrightness);
            prefs.end();
        }
#else
        std::string fname = "zic_proj_" + std::to_string(slotIdx) + ".dat";
        std::ofstream ofs(fname, std::ios::binary);
        if (ofs.is_open()) {
            ofs.write(reinterpret_cast<const char*>(&pData), sizeof(pData));
        }
        std::ofstream metaOfs("zic_meta.dat", std::ios::binary);
        if (metaOfs.is_open()) {
            metaOfs.write(reinterpret_cast<const char*>(&currentProject), sizeof(currentProject));
            metaOfs.write(reinterpret_cast<const char*>(&padBrightness), sizeof(padBrightness));
        }
#endif
    }

    void loadAllFromStorage()
    {
#ifdef ARDUINO
        Preferences prefs;
        if (prefs.begin("zicApp", true)) {
            uint8_t currP = prefs.getUChar("curr_p", 255);
            uint8_t bVal = prefs.getUChar("pad_bright", 0);
            if (bVal < 4) padBrightness = bVal;
            if (currP != 255 && currP < 16) {
                for (int i = 0; i < 16; ++i) {
                    char key[16];
                    snprintf(key, sizeof(key), "p_%d", i);
                    PersistedProjectSlot pData;
                    if (prefs.getBytes(key, &pData, sizeof(pData)) == sizeof(pData)) {
                        unpackProjectSlot(pData, projects[i]);
                    }
                }
                currentProject = currP;
                brain.bpm = projects[currentProject].bpm;
                brain.updateTiming();
                masterVolume = projects[currentProject].masterVolume;
                brain.selectedTrack = projects[currentProject].selectedTrack;
                std::memcpy(brain.tracks, projects[currentProject].tracks, sizeof(brain.tracks));
                std::memcpy(sampleTracks, projects[currentProject].sampleTracks, sizeof(sampleTracks));
            }
            prefs.end();
        }
#else
        std::ifstream metaIfs("zic_meta.dat", std::ios::binary);
        if (metaIfs.is_open()) {
            uint8_t currP = 0;
            metaIfs.read(reinterpret_cast<char*>(&currP), sizeof(currP));
            uint8_t bVal = 0;
            if (metaIfs.read(reinterpret_cast<char*>(&bVal), sizeof(bVal))) {
                if (bVal < 4) padBrightness = bVal;
            }
            if (currP < 16) {
                for (int i = 0; i < 16; ++i) {
                    std::string fname = "zic_proj_" + std::to_string(i) + ".dat";
                    std::ifstream ifs(fname, std::ios::binary);
                    if (ifs.is_open()) {
                        PersistedProjectSlot pData;
                        ifs.read(reinterpret_cast<char*>(&pData), sizeof(pData));
                        unpackProjectSlot(pData, projects[i]);
                    }
                }
                currentProject = currP;
                brain.bpm = projects[currentProject].bpm;
                brain.updateTiming();
                masterVolume = projects[currentProject].masterVolume;
                brain.selectedTrack = projects[currentProject].selectedTrack;
                std::memcpy(brain.tracks, projects[currentProject].tracks, sizeof(brain.tracks));
                std::memcpy(sampleTracks, projects[currentProject].sampleTracks, sizeof(sampleTracks));
            }
        }
#endif
    }

    void saveTrackToClip(int t, int clipIdx)
    {
        if (t < 0 || t >= SequenceBrain::NUM_TRACKS || clipIdx < 0 || clipIdx >= 8) return;
        Clip& c = brain.tracks[t].clips[clipIdx];
        c.isCreated = true;
        c.sampleIdx = sampleTracks[t].sampleIdx;
        c.pitch = sampleTracks[t].pitch;
        c.volume = sampleTracks[t].volume;
        std::memcpy(c.steps, brain.tracks[t].steps, sizeof(brain.tracks[t].steps));
    }

    void loadTrackFromClip(int t, int clipIdx)
    {
        if (t < 0 || t >= SequenceBrain::NUM_TRACKS || clipIdx < 0 || clipIdx >= 8) return;
        saveTrackToClip(t, brain.tracks[t].activeClip);

        brain.tracks[t].activeClip = clipIdx;
        Clip& c = brain.tracks[t].clips[clipIdx];
        if (!c.isCreated) {
            c.isCreated = true;
            c.sampleIdx = t;
            c.pitch = 0.0f;
            c.volume = 1.0f;
            for (int s = 0; s < SequenceBrain::NUM_STEPS; ++s) {
                c.steps[s].active = false;
                c.steps[s].note = 60;
                c.steps[s].velocity = 0.8f;
                c.steps[s].probability = 100;
            }
        }

        sampleTracks[t].setSample(c.sampleIdx);
        sampleTracks[t].pitch = c.pitch;
        sampleTracks[t].updateSpeed();
        sampleTracks[t].volume = c.volume;
        std::memcpy(brain.tracks[t].steps, c.steps, sizeof(c.steps));
        markDirty();
    }

    void saveCurrentProjectSlot(uint8_t slotIdx)
    {
        if (slotIdx >= 16) return;
        for (int t = 0; t < SequenceBrain::NUM_TRACKS; ++t) {
            saveTrackToClip(t, brain.tracks[t].activeClip);
        }
        projects[slotIdx].isOccupied = true;
        projects[slotIdx].bpm = brain.bpm;
        projects[slotIdx].masterVolume = masterVolume;
        projects[slotIdx].selectedTrack = brain.selectedTrack;
        std::memcpy(projects[slotIdx].tracks, brain.tracks, sizeof(brain.tracks));
        std::memcpy(projects[slotIdx].sampleTracks, sampleTracks, sizeof(sampleTracks));
    }

    void initEmptyProjectSlot(uint8_t slotIdx)
    {
        if (slotIdx >= 16) return;

        projects[slotIdx].isOccupied = true;
        projects[slotIdx].bpm = 125.0f;
        projects[slotIdx].masterVolume = 1.0f;
        projects[slotIdx].selectedTrack = 0;

        for (int t = 0; t < SequenceBrain::NUM_TRACKS; ++t) {
            projects[slotIdx].tracks[t].name = brain.tracks[t].name;
            projects[slotIdx].tracks[t].muted = false;
            projects[slotIdx].tracks[t].activeClip = 0;

            for (int s = 0; s < SequenceBrain::NUM_STEPS; ++s) {
                projects[slotIdx].tracks[t].steps[s].active = false;
                projects[slotIdx].tracks[t].steps[s].note = 60;
                projects[slotIdx].tracks[t].steps[s].velocity = 0.8f;
                projects[slotIdx].tracks[t].steps[s].probability = 100;
            }

            for (int c = 0; c < 8; ++c) {
                projects[slotIdx].tracks[t].clips[c].isCreated = false;
            }

            projects[slotIdx].tracks[t].clips[0].isCreated = true;
            projects[slotIdx].tracks[t].clips[0].sampleIdx = t;
            projects[slotIdx].tracks[t].clips[0].pitch = 0.0f;
            projects[slotIdx].tracks[t].clips[0].volume = 1.0f;
            std::memcpy(projects[slotIdx].tracks[t].clips[0].steps, projects[slotIdx].tracks[t].steps, sizeof(projects[slotIdx].tracks[t].steps));

            projects[slotIdx].sampleTracks[t].init(t, brain.tracks[t].name, t);
            projects[slotIdx].sampleTracks[t].pitch = 0.0f;
            projects[slotIdx].sampleTracks[t].volume = 1.0f;
        }
    }

    void loadProjectSlot(uint8_t slotIdx)
    {
        if (slotIdx >= 16) return;
        saveCurrentProjectSlot(currentProject);

        currentProject = slotIdx;
        if (!projects[slotIdx].isOccupied) {
            initEmptyProjectSlot(slotIdx);
        }

        brain.bpm = projects[slotIdx].bpm;
        brain.updateTiming();
        masterVolume = projects[slotIdx].masterVolume;
        brain.selectedTrack = projects[slotIdx].selectedTrack;
        std::memcpy(brain.tracks, projects[slotIdx].tracks, sizeof(brain.tracks));
        std::memcpy(sampleTracks, projects[slotIdx].sampleTracks, sizeof(sampleTracks));
        markDirty();
    }

    void copyProjectSlot(uint8_t srcIdx, uint8_t dstIdx)
    {
        if (srcIdx >= 16 || dstIdx >= 16) return;
        if (srcIdx == currentProject) {
            saveCurrentProjectSlot(srcIdx);
        }
        projects[dstIdx] = projects[srcIdx];
        projects[dstIdx].isOccupied = true;
        markDirty();
    }

    void setView(int viewIdx)
    {
        int v = (viewIdx % NUM_VIEWS + NUM_VIEWS) % NUM_VIEWS;
        currentView = (ViewMode)v;
        showProbSubMenu = false;
        globalMenuMode = false;
        copyState = COPY_IDLE;
        copySourcePad = -1;
        copyTargetPad = -1;
    }

    void nextView()
    {
        setView((int)currentView + 1);
    }

    void prevView()
    {
        setView((int)currentView - 1);
    }

    void selectTrack(int trkIdx)
    {
        brain.selectedTrack = (trkIdx + SequenceBrain::NUM_TRACKS) % SequenceBrain::NUM_TRACKS;
        if (!brain.isPlaying || autoTriggerOnSelect) {
            sampleTracks[brain.selectedTrack].trigger();
        }
    }

    void processMidiByte(uint8_t byte)
    {
        uint32_t nowMs = getSystemMillis();
        if (byte == 0xF8) { // MIDI Realtime Clock (24 PPQN -> 6 ticks per 16th step)
            lastMidiClockMs = nowMs;
            isExternalClock = true;
            midiTickCounter++;
            if (midiTickCounter >= 6) {
                midiTickCounter = 0;
                pendingStepTriggers++;
            }
        } else if (byte == 0xFA) { // MIDI Start
            lastMidiClockMs = nowMs;
            isExternalClock = true;
            brain.isPlaying = true;
            brain.currentStep = SequenceBrain::NUM_STEPS - 1; // So next step is 0
            midiTickCounter = 5;                              // Next clock tick triggers step 0 immediately
            pendingStepTriggers = 0;
        } else if (byte == 0xFC) { // MIDI Stop
            brain.isPlaying = false;
            pendingStepTriggers = 0;
            midiTickCounter = 0;
        } else if (byte == 0xFB) { // MIDI Continue
            lastMidiClockMs = nowMs;
            isExternalClock = true;
            brain.isPlaying = true;
        }
    }

    void triggerCurrentStepVoices()
    {
        for (int t = 0; t < SequenceBrain::NUM_TRACKS; ++t) {
            if (!brain.tracks[t].muted && brain.tracks[t].steps[brain.currentStep].active) {
                uint8_t prob = brain.tracks[t].steps[brain.currentStep].probability;
                if (prob >= 100 || (rand() % 100) < prob) {
                    sampleTracks[t].trigger();
                }
            }
        }
    }

    // Periodically checked (in main loop / desktop loop) to trigger long press as soon as 400ms elapses
    void updateHoldTimers(uint32_t nowMs = 0)
    {
        if (nowMs == 0) nowMs = getSystemMillis();

        // Automatic Fallback to Internal Clock if no external MIDI clock received for 500ms
        if (isExternalClock && (nowMs >= lastMidiClockMs) && (nowMs - lastMidiClockMs >= 500)) {
            isExternalClock = false;
            midiTickCounter = 0;
            pendingStepTriggers = 0;
        }

        if (isDirty && (nowMs - lastChangeTime >= 1000)) {
            saveProjectSlotToStorage(currentProject);
            isDirty = false;
        }

        if (currentView == VIEW_STEP_EDIT && !showProbSubMenu) {
            for (int i = 0; i < 16; ++i) {
                if (padIsDown[i] && !padLongPressHandled[i]) {
                    uint32_t duration = (nowMs >= padPressTime[i]) ? (nowMs - padPressTime[i]) : 0;
                    if (duration >= 400) { // 400ms reached while holding pad!
                        showProbSubMenu = true;
                        probEditingStep = i;
                        brain.tracks[brain.selectedTrack].steps[i].active = true;
                        padLongPressHandled[i] = true;
                        markDirty();
                        break;
                    }
                }
            }
        } else if (currentView == VIEW_PROJECTS) {
            for (int i = 0; i < 16; ++i) {
                if (padIsDown[i] && !padLongPressHandled[i]) {
                    uint32_t duration = (nowMs >= padPressTime[i]) ? (nowMs - padPressTime[i]) : 0;
                    if (duration >= 400) { // 400ms long press on Project pad
                        if (copyState == COPY_IDLE) {
                            saveCurrentProjectSlot(currentProject);
                            copySourcePad = i;
                            copyState = COPY_WAIT_TARGET;
                        }
                        padLongPressHandled[i] = true;
                        break;
                    }
                }
            }
        }
    }

    // Key / NeoTrellis Press & Release Event Handler
    void handlePadEvent(int padIdx, bool isPress, uint32_t nowMs = 0)
    {
        if (padIdx < 0 || padIdx >= 16) return;
        if (nowMs == 0) nowMs = getSystemMillis();

        if (isPress) {
            padPressTime[padIdx] = nowMs;
            padIsDown[padIdx] = true;
            padLongPressHandled[padIdx] = false;

            if (showProbSubMenu) {
                // Select probability preset and close sub-menu immediately
                brain.tracks[brain.selectedTrack].steps[probEditingStep].probability = PROBABILITY_PRESETS[padIdx];
                showProbSubMenu = false;
                padLongPressHandled[padIdx] = true; // Prevent release event from toggling step
                markDirty();
                return;
            }

            if (currentView != VIEW_STEP_EDIT && currentView != VIEW_PROJECTS) {
                handlePadPress(padIdx, true);
            }
        } else {
            // Release event
            padIsDown[padIdx] = false;

            if (padLongPressHandled[padIdx]) {
                padLongPressHandled[padIdx] = false;
                return;
            }

            if (showProbSubMenu) return;

            if (currentView == VIEW_STEP_EDIT) {
                uint32_t duration = (nowMs >= padPressTime[padIdx]) ? (nowMs - padPressTime[padIdx]) : 0;
                if (duration < 400) { // Short Press (< 400ms): Toggle Step
                    brain.toggleStep(brain.selectedTrack, padIdx);
                    markDirty();
                }
            } else if (currentView == VIEW_PROJECTS) {
                uint32_t duration = (nowMs >= padPressTime[padIdx]) ? (nowMs - padPressTime[padIdx]) : 0;
                if (duration < 400) { // Short Press on Project View
                    handlePadPress(padIdx, true);
                }
            }
        }
    }

    // NeoTrellis / Keyboard / Touch input handler
    void handlePadPress(int padIdx, bool pressed)
    {
        if (!pressed) return;

        if (currentView == VIEW_STEP_EDIT) {
            if (showProbSubMenu) {
                brain.tracks[brain.selectedTrack].steps[probEditingStep].probability = PROBABILITY_PRESETS[padIdx];
                showProbSubMenu = false;
                markDirty();
            } else {
                brain.toggleStep(brain.selectedTrack, padIdx);
                markDirty();
            }
        } else if (currentView == VIEW_CLIPS) {
            if (padIdx >= 0 && padIdx < 8) {
                selectTrack(padIdx);
            } else if (padIdx >= 8 && padIdx < 16) {
                loadTrackFromClip(brain.selectedTrack, padIdx - 8);
            }
        } else if (currentView == VIEW_PROJECTS) {
            if (copyState == COPY_IDLE) {
                loadProjectSlot(padIdx);
            } else if (copyState == COPY_WAIT_TARGET) {
                if (padIdx == copySourcePad) {
                    // Cancel copy by pressing same pad again
                    copyState = COPY_IDLE;
                    copySourcePad = -1;
                } else if (!projects[padIdx].isOccupied) {
                    // Paste directly into empty pad
                    copyProjectSlot(copySourcePad, padIdx);
                    copyState = COPY_IDLE;
                    copySourcePad = -1;
                } else {
                    // Target is occupied -> Ask confirmation to overwrite
                    copyTargetPad = padIdx;
                    copyState = COPY_CONFIRM_OVERWRITE;
                }
            } else if (copyState == COPY_CONFIRM_OVERWRITE) {
                if (padIdx == copyTargetPad) {
                    // Press target pad again -> Confirm overwrite!
                    copyProjectSlot(copySourcePad, copyTargetPad);
                    copyState = COPY_IDLE;
                    copySourcePad = -1;
                    copyTargetPad = -1;
                } else {
                    // Press any other pad -> Cancel copy!
                    copyState = COPY_IDLE;
                    copySourcePad = -1;
                    copyTargetPad = -1;
                }
            }
        } else if (currentView == VIEW_OVERVIEW) {
            if (padIdx >= 0 && padIdx < 8) {
                selectTrack(padIdx);
            } else if (padIdx >= 8 && padIdx < 16) {
                brain.toggleMute(padIdx - 8);
                markDirty();
            }
        } else if (currentView == VIEW_SOUND_EDIT) {
            if (padIdx >= 0 && padIdx < 8) {
                selectTrack(padIdx);
            } else {
                int trk = brain.selectedTrack;
                SampleTrack& sTrk = sampleTracks[trk];
                uint8_t numPcm = sizeof(g_presetSamples) / sizeof(g_presetSamples[0]);

                if (padIdx == 8) { // 'A': Sample -
                    sTrk.setSample((sTrk.sampleIdx + numPcm - 1) % numPcm);
                    if (!brain.isPlaying) sTrk.trigger();
                    markDirty();
                } else if (padIdx == 12) { // 'Z': Sample +
                    sTrk.setSample((sTrk.sampleIdx + 1) % numPcm);
                    if (!brain.isPlaying) sTrk.trigger();
                    markDirty();
                } else if (padIdx == 9) { // 'S': Pitch -
                    sTrk.pitch = std::clamp(sTrk.pitch - 1.0f, -12.0f, 12.0f);
                    sTrk.updateSpeed();
                    if (!brain.isPlaying) sTrk.trigger();
                    markDirty();
                } else if (padIdx == 13) { // 'X': Pitch +
                    sTrk.pitch = std::clamp(sTrk.pitch + 1.0f, -12.0f, 12.0f);
                    sTrk.updateSpeed();
                    if (!brain.isPlaying) sTrk.trigger();
                    markDirty();
                } else if (padIdx == 10) { // 'D': Volume -
                    sTrk.volume = std::clamp(sTrk.volume - 0.1f, 0.0f, 2.0f);
                    if (!brain.isPlaying) sTrk.trigger();
                    markDirty();
                } else if (padIdx == 14) { // 'C': Volume + (up to 200% Gain)
                    sTrk.volume = std::clamp(sTrk.volume + 0.1f, 0.0f, 2.0f);
                    if (!brain.isPlaying) sTrk.trigger();
                    markDirty();
                } else if (padIdx == 11) { // 'F': Manual Trigger (Always triggers)
                    sTrk.trigger();
                } else if (padIdx == 15) { // 'V': Mute toggle
                    brain.toggleMute(trk);
                    markDirty();
                }
            }
        } else if (currentView == VIEW_GLOBAL) {
            if (globalMenuMode) {
                if (padIdx == 0) { // Pad 0: Toggle Trigger on Select
                    autoTriggerOnSelect = !autoTriggerOnSelect;
                    markDirty();
                } else if (padIdx == 1) { // Pad 1: Cycle Brightness
                    cycleBrightness();
                } else if (padIdx == 8) { // Pad 8 / 'A': Close Menu
                    globalMenuMode = false;
                } else if (padIdx == 10) { // 'D': BPM -5
                    brain.setBpm(brain.bpm - 5.0f);
                    markDirty();
                } else if (padIdx == 11) { // 'F': Master Volume -
                    masterVolume = std::clamp(masterVolume - 0.1f, 0.0f, 2.0f);
                    markDirty();
                } else if (padIdx == 12) { // 'Z': Play / Pause
                    brain.isPlaying = !brain.isPlaying;
                } else if (padIdx == 14) { // 'C': BPM +5
                    brain.setBpm(brain.bpm + 5.0f);
                    markDirty();
                } else if (padIdx == 15) { // 'V': Master Volume +
                    masterVolume = std::clamp(masterVolume + 0.1f, 0.0f, 2.0f);
                    markDirty();
                }
            } else {
                if (padIdx >= 0 && padIdx < 8) {
                    selectTrack(padIdx);
                } else if (padIdx == 8) { // Pad 8 / 'A': Open Menu
                    globalMenuMode = true;
                } else if (padIdx == 10) { // 'D': BPM -5
                    brain.setBpm(brain.bpm - 5.0f);
                    markDirty();
                } else if (padIdx == 11) { // 'F': Master Volume -
                    masterVolume = std::clamp(masterVolume - 0.1f, 0.0f, 2.0f);
                    markDirty();
                } else if (padIdx == 12) { // 'Z': Play / Pause
                    brain.isPlaying = !brain.isPlaying;
                } else if (padIdx == 14) { // 'C': BPM +5
                    brain.setBpm(brain.bpm + 5.0f);
                    markDirty();
                } else if (padIdx == 15) { // 'V': Master Volume +
                    masterVolume = std::clamp(masterVolume + 0.1f, 0.0f, 2.0f);
                    markDirty();
                }
            }
        }
    }

    float renderMasterSample()
    {
        // 1. Advance sequencer step timing
        if (brain.isPlaying) {
            if (isExternalClock) {
                if (pendingStepTriggers > 0) {
                    pendingStepTriggers--;
                    brain.currentStep = (brain.currentStep + 1) % SequenceBrain::NUM_STEPS;
                    triggerCurrentStepVoices();
                }
            } else {
                brain.sampleCounter++;
                if (brain.sampleCounter >= (uint64_t)brain.samplesPerStep) {
                    brain.sampleCounter = 0;
                    brain.currentStep = (brain.currentStep + 1) % SequenceBrain::NUM_STEPS;
                    triggerCurrentStepVoices();
                }
            }
        }

        // 2. Mix 8 sample tracks
        float mix = 0.0f;
        for (int t = 0; t < SequenceBrain::NUM_TRACKS; ++t) {
            mix += sampleTracks[t].renderNextSample();
        }

        return std::clamp(mix * masterVolume, -1.0f, 1.0f);
    }
};
