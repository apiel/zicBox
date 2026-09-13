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
    bool autoTriggerOnSelect = true;

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

    void selectTrack(int trkIdx)
    {
        brain.selectedTrack = (trkIdx + SequenceBrain::NUM_TRACKS) % SequenceBrain::NUM_TRACKS;
        if (!brain.isPlaying || autoTriggerOnSelect) {
            sampleTracks[brain.selectedTrack].trigger();
        }
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
                selectTrack(padIdx);
            } else if (padIdx >= 8 && padIdx < 16) {
                brain.toggleMute(padIdx - 8);
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
                } else if (padIdx == 12) { // 'Z': Sample +
                    sTrk.setSample((sTrk.sampleIdx + 1) % numPcm);
                    if (!brain.isPlaying) sTrk.trigger();
                } else if (padIdx == 9) { // 'S': Pitch -
                    sTrk.pitch = std::clamp(sTrk.pitch - 1.0f, -12.0f, 12.0f);
                    sTrk.updateSpeed();
                    if (!brain.isPlaying) sTrk.trigger();
                } else if (padIdx == 13) { // 'X': Pitch +
                    sTrk.pitch = std::clamp(sTrk.pitch + 1.0f, -12.0f, 12.0f);
                    sTrk.updateSpeed();
                    if (!brain.isPlaying) sTrk.trigger();
                } else if (padIdx == 10) { // 'D': Volume -
                    sTrk.volume = std::clamp(sTrk.volume - 0.1f, 0.0f, 2.0f);
                    if (!brain.isPlaying) sTrk.trigger();
                } else if (padIdx == 14) { // 'C': Volume + (up to 200% Gain)
                    sTrk.volume = std::clamp(sTrk.volume + 0.1f, 0.0f, 2.0f);
                    if (!brain.isPlaying) sTrk.trigger();
                } else if (padIdx == 11) { // 'F': Manual Trigger (Always triggers)
                    sTrk.trigger();
                } else if (padIdx == 15) { // 'V': Mute toggle
                    brain.toggleMute(trk);
                }
            }
        } else if (currentView == VIEW_GLOBAL) {
            if (padIdx >= 0 && padIdx < 8) {
                selectTrack(padIdx);
            } else if (padIdx == 8) { // 'A': Toggle Trigger on Track Select during playback
                autoTriggerOnSelect = !autoTriggerOnSelect;
            } else if (padIdx == 10) { // 'D': BPM -5
                brain.setBpm(brain.bpm - 5.0f);
            } else if (padIdx == 11) { // 'F': Master Volume -
                masterVolume = std::clamp(masterVolume - 0.1f, 0.0f, 2.0f);
            } else if (padIdx == 12) { // 'Z': Play / Pause
                brain.isPlaying = !brain.isPlaying;
            } else if (padIdx == 14) { // 'C': BPM +5
                brain.setBpm(brain.bpm + 5.0f);
            } else if (padIdx == 15) { // 'V': Master Volume + (up to 200%)
                masterVolume = std::clamp(masterVolume + 0.1f, 0.0f, 2.0f);
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
