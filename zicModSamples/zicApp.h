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
inline uint32_t getSystemMillis() { return millis(); }
#else
inline uint32_t getSystemMillis() {
    using namespace std::chrono;
    return (uint32_t)duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
#endif

enum ViewMode {
    VIEW_OVERVIEW = 0,
    VIEW_STEP_EDIT = 1,
    VIEW_SOUND_EDIT = 2,
    VIEW_GLOBAL = 3,
    NUM_VIEWS = 4
};

static const uint8_t PROBABILITY_PRESETS[16] = {
    0,  10, 20, 25,
    33, 40, 50, 60,
    66, 70, 75, 80,
    85, 90, 95, 100
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

    // Step Probability Sub-Menu State
    bool showProbSubMenu = false;
    uint8_t probEditingStep = 0;
    uint32_t padPressTime[16] = { 0 };
    bool padIsDown[16] = { false };
    bool padLongPressHandled[16] = { false };

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
        showProbSubMenu = false;
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

    // Periodically checked (in main loop / desktop loop) to trigger long press as soon as 400ms elapses
    void updateHoldTimers(uint32_t nowMs = 0)
    {
        if (nowMs == 0) nowMs = getSystemMillis();

        if (currentView == VIEW_STEP_EDIT && !showProbSubMenu) {
            for (int i = 0; i < 16; ++i) {
                if (padIsDown[i] && !padLongPressHandled[i]) {
                    uint32_t duration = (nowMs >= padPressTime[i]) ? (nowMs - padPressTime[i]) : 0;
                    if (duration >= 400) { // 400ms reached while holding pad!
                        showProbSubMenu = true;
                        probEditingStep = i;
                        brain.tracks[brain.selectedTrack].steps[i].active = true;
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
                return;
            }

            if (currentView != VIEW_STEP_EDIT) {
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
            } else {
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

                // Trigger active steps across all 8 tracks based on step probability
                for (int t = 0; t < SequenceBrain::NUM_TRACKS; ++t) {
                    if (!brain.tracks[t].muted && brain.tracks[t].steps[brain.currentStep].active) {
                        uint8_t prob = brain.tracks[t].steps[brain.currentStep].probability;
                        if (prob >= 100 || (rand() % 100) < prob) {
                            sampleTracks[t].trigger();
                        }
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
