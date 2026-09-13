#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "audio/engines/DrumClap.h"
#include "audio/engines/DrumHiHat23.h"
#include "audio/engines/DrumPercussion.h"
#include "audio/engines/DrumSnare.h"
#include "sequenceBrain.h"

enum ViewMode {
    VIEW_OVERVIEW = 0,
    VIEW_STEP_EDIT = 1,
    VIEW_SOUND_EDIT = 2,
    VIEW_GLOBAL = 3,
    NUM_VIEWS = 4
};

class ZicApp {
public:
    float sampleRate = 44100.0f;

    // Audio FX shared buffers
    float rvBuffer[22050] = { 0.0f };

    // 4 Drum synthesis engines (No Kick - Snare, Clap, HiHat, Perc)
    DrumSnare snareEngine;
    DrumClap clapEngine;
    DrumHiHat23 hihatEngine;
    DrumPercussion percEngine;

    // Sequencer brain
    SequenceBrain brain;

    // Application State
    ViewMode currentView = VIEW_OVERVIEW;
    float masterVolume = 0.8f;
    int activeParamIdx = 0; // selected parameter index in sound edit view

    ZicApp(float sampleRate = 44100.0f)
        : sampleRate(sampleRate)
        , snareEngine(sampleRate)
        , clapEngine(sampleRate, rvBuffer)
        , hihatEngine(sampleRate, rvBuffer)
        , percEngine(sampleRate, rvBuffer)
        , brain(sampleRate)
    {
        // Bind engines to sequencer tracks
        brain.tracks[0].engine = &snareEngine;
        brain.tracks[1].engine = &clapEngine;
        brain.tracks[2].engine = &hihatEngine;
        brain.tracks[3].engine = &percEngine;
    }

    void setView(int viewIdx)
    {
        int v = (viewIdx + NUM_VIEWS) % NUM_VIEWS;
        currentView = (ViewMode)v;
    }

    void nextView()
    {
        setView((int)currentView + 1);
    }

    void prevView()
    {
        setView((int)currentView - 1);
    }

    IEngine* getSelectedEngine()
    {
        return brain.tracks[brain.selectedTrack].engine;
    }

    void selectNextParam()
    {
        IEngine* eng = getSelectedEngine();
        if (!eng) return;
        int count = (int)eng->getParamCount();
        if (count == 0) return;
        activeParamIdx = (activeParamIdx + 1) % count;
    }

    void selectPrevParam()
    {
        IEngine* eng = getSelectedEngine();
        if (!eng) return;
        int count = (int)eng->getParamCount();
        if (count == 0) return;
        activeParamIdx = (activeParamIdx - 1 + count) % count;
    }

    void adjustSelectedParam(float delta)
    {
        IEngine* eng = getSelectedEngine();
        if (!eng) return;
        int count = (int)eng->getParamCount();
        if (count == 0) return;

        activeParamIdx = std::clamp(activeParamIdx, 0, count - 1);
        Param* params = eng->getParams();
        Param& p = params[activeParamIdx];

        float step = p.step > 0.0f ? p.step : ((p.max - p.min) * 0.05f);
        p.set(std::clamp(p.value + delta * step, p.min, p.max));
    }

    // NeoTrellis / Keyboard input handler (Pads 0..15)
    void handlePadPress(int padIdx, bool pressed)
    {
        if (!pressed) return; // Trigger on press

        if (currentView == VIEW_STEP_EDIT) {
            // Step View: Pads 0..15 toggle steps 0..15 for selected track
            if (padIdx >= 0 && padIdx < 16) {
                brain.toggleStep(brain.selectedTrack, padIdx);
            }
        } else if (currentView == VIEW_OVERVIEW) {
            // Overview View:
            // Row 0 (Pads 0..3): Select track 0..3
            // Row 1 (Pads 4..7): Toggle Mute for track 0..3
            // Row 3 (Pads 12..15): Play/Stop, BPM-, BPM+, Pattern Gen
            if (padIdx >= 0 && padIdx < 4) {
                brain.selectedTrack = padIdx;
                activeParamIdx = 0;
            } else if (padIdx >= 4 && padIdx < 8) {
                brain.toggleMute(padIdx - 4);
            } else if (padIdx == 12) {
                brain.isPlaying = !brain.isPlaying;
            } else if (padIdx == 13) {
                brain.setBpm(brain.bpm - 5.0f);
            } else if (padIdx == 14) {
                brain.setBpm(brain.bpm + 5.0f);
            } else if (padIdx == 15) {
                brain.generatePattern();
            }
        } else if (currentView == VIEW_SOUND_EDIT) {
            // Sound Edit View:
            // Pads 0..3: Select tracks 0..3
            // Pads 4..7: Select parameter 0..3 on current visible page
            // Pads 8..9: Previous / Next parameter page
            // Pads 10..11: Adjust parameter - / +
            if (padIdx >= 0 && padIdx < 4) {
                brain.selectedTrack = padIdx;
                activeParamIdx = 0;
            } else if (padIdx >= 4 && padIdx < 8) {
                int pageOffset = (activeParamIdx / 4) * 4;
                IEngine* eng = getSelectedEngine();
                if (eng) {
                    int count = (int)eng->getParamCount();
                    activeParamIdx = std::clamp(pageOffset + (padIdx - 4), 0, count - 1);
                }
            } else if (padIdx == 8) {
                selectPrevParam();
            } else if (padIdx == 9) {
                selectNextParam();
            } else if (padIdx == 10) {
                adjustSelectedParam(-1.0f);
            } else if (padIdx == 11) {
                adjustSelectedParam(1.0f);
            }
        } else if (currentView == VIEW_GLOBAL) {
            if (padIdx == 0) brain.isPlaying = !brain.isPlaying;
            else if (padIdx == 1) brain.setBpm(brain.bpm - 5.0f);
            else if (padIdx == 2) brain.setBpm(brain.bpm + 5.0f);
            else if (padIdx == 3) brain.generatePattern();
            else if (padIdx >= 4 && padIdx < 8) {
                brain.selectedTrack = padIdx - 4;
                activeParamIdx = 0;
            }
        }
    }

    float renderMasterSample()
    {
        float sample = brain.renderAudioSample();
        return std::clamp(sample * masterVolume, -1.0f, 1.0f);
    }
};
