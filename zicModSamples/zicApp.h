#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "SampleTrack.h"
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

    // 8 Sample Playback Tracks
    SampleTrack sampleTracks[8];

    // Sequencer brain
    SequenceBrain brain;

    // Application State
    ViewMode currentView = VIEW_OVERVIEW;
    float masterVolume = 1.0f;

    ZicApp(float sampleRate = 44100.0f)
        : sampleRate(sampleRate)
        , brain(sampleRate)
    {
        // Initialize 8 sample tracks with default preset samples
        sampleTracks[0].init(0, "Snare", 0);
        sampleTracks[1].init(1, "Clap", 1);
        sampleTracks[2].init(2, "HatCl", 2);
        sampleTracks[3].init(3, "HatOp", 3);
        sampleTracks[4].init(4, "Perc1", 4);
        sampleTracks[5].init(5, "Perc2", 5);
        sampleTracks[6].init(6, "Tom", 6);
        sampleTracks[7].init(7, "FX", 7);
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

    // NeoTrellis / Keyboard / Touch input handler
    void handlePadPress(int padIdx, bool pressed)
    {
        if (!pressed) return;

        if (currentView == VIEW_STEP_EDIT) {
            if (padIdx >= 0 && padIdx < 16) {
                brain.toggleStep(brain.selectedTrack, padIdx);
            }
        } else if (currentView == VIEW_OVERVIEW) {
            if (padIdx >= 0 && padIdx < 8) {
                brain.selectedTrack = padIdx;
                sampleTracks[padIdx].trigger();
            } else if (padIdx >= 8 && padIdx < 16) {
                brain.toggleMute(padIdx - 8);
            }
        } else if (currentView == VIEW_SOUND_EDIT) {
            if (padIdx >= 0 && padIdx < 8) {
                brain.selectedTrack = padIdx;
                sampleTracks[padIdx].trigger();
            }
        } else if (currentView == VIEW_GLOBAL) {
            if (padIdx == 0) brain.isPlaying = !brain.isPlaying;
            else if (padIdx == 1) brain.setBpm(brain.bpm - 5.0f);
            else if (padIdx == 2) brain.setBpm(brain.bpm + 5.0f);
            else if (padIdx == 3) brain.generatePattern();
            else if (padIdx >= 4 && padIdx < 12) {
                brain.selectedTrack = padIdx - 4;
            }
        }
    }

    float renderMasterSample()
    {
        // 1. Advance sequencer step timing
        if (brain.isPlaying) {
            brain.sampleCounter++;
            if (brain.sampleCounter >= (uint64_t)brain.samplesPerStep) {
                brain.sampleCounter = 0;
                brain.currentStep = (brain.currentStep + 1) % SequenceBrain::NUM_STEPS;

                // Trigger active steps across all 8 tracks
                for (int t = 0; t < SequenceBrain::NUM_TRACKS; ++t) {
                    if (!brain.tracks[t].muted && brain.tracks[t].steps[brain.currentStep].active) {
                        sampleTracks[t].trigger();
                    }
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
