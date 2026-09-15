#pragma once

#include <Adafruit_NeoTrellis.h>
#include <Arduino.h>
#include "../../zicApp.h"

// Global pointer to app for NeoTrellis callbacks
static ZicApp* g_neoAppPtr = nullptr;

static TrellisCallback neoTrellisCallback(keyEvent evt)
{
    if (!g_neoAppPtr) return 0;
    int padIdx = evt.bit.NUM;
    bool isPress = (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING);
    g_neoAppPtr->handlePadEvent(padIdx, isPress);
    return 0;
}

struct NeoRGB { uint8_t r, g, b; };
static const NeoRGB NEO_TRACK_COLORS[8] = {
    { 0, 210, 255 },   // Trk 0: Electric Cyan
    { 255, 0, 127 },   // Trk 1: Hot Pink
    { 255, 208, 0 },   // Trk 2: Amber Yellow
    { 0, 255, 102 },   // Trk 3: Neon Green
    { 255, 102, 0 },   // Trk 4: Vibrant Orange
    { 153, 51, 255 },  // Trk 5: Violet Purple
    { 255, 51, 51 },   // Trk 6: Bright Red
    { 0, 255, 204 }    // Trk 7: Bright Teal
};

class NeoTrellisESP32 {
public:
    Adafruit_NeoTrellis trellis = Adafruit_NeoTrellis(0x2E);
    bool initialized = false;

    void init(ZicApp* app)
    {
        g_neoAppPtr = app;
        if (trellis.begin()) {
            initialized = true;
            for (int i = 0; i < 16; i++) {
                trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
                trellis.activateKey(i, SEESAW_KEYPAD_EDGE_FALLING);
                trellis.registerCallback(i, neoTrellisCallback);
            }
        }
    }

    void update(ZicApp& app)
    {
        if (!initialized) return;
        trellis.read();
        trellis.pixels.setBrightness(app.getNeoBrightnessValue());

        // Update LED feedback on NeoTrellis matrix to match on-screen UI
        for (int i = 0; i < 16; i++) {
            if (app.currentView == VIEW_STEP_EDIT) {
                if (app.showProbSubMenu) {
                    uint8_t currProb = app.brain.tracks[app.brain.selectedTrack].steps[app.probEditingStep].probability;
                    uint8_t presetVal = PROBABILITY_PRESETS[i];
                    if (presetVal == currProb) {
                        trellis.pixels.setPixelColor(i, trellis.pixels.Color(255, 255, 255));
                    } else if (presetVal == 100) {
                        trellis.pixels.setPixelColor(i, trellis.pixels.Color(0, 180, 90));
                    } else if (presetVal == 0) {
                        trellis.pixels.setPixelColor(i, trellis.pixels.Color(150, 40, 40));
                    } else {
                        trellis.pixels.setPixelColor(i, trellis.pixels.Color(0, 140, 220));
                    }
                } else {
                    int trk = app.brain.selectedTrack;
                    bool active = app.brain.tracks[trk].steps[i].active;
                    bool isCurrent = (app.brain.currentStep == i && app.brain.isPlaying);

                    if (isCurrent) {
                        trellis.pixels.setPixelColor(i, trellis.pixels.Color(255, 255, 255));
                    } else if (active) {
                        NeoRGB c = NEO_TRACK_COLORS[trk];
                        trellis.pixels.setPixelColor(i, trellis.pixels.Color(c.r, c.g, c.b));
                    } else {
                        trellis.pixels.setPixelColor(i, trellis.pixels.Color(4, 6, 10));
                    }
                }
            } else if (app.currentView == VIEW_OVERVIEW) {
                if (i < 8) { // Row 0 & 1: Track select
                    bool isSel = (app.brain.selectedTrack == i);
                    NeoRGB c = NEO_TRACK_COLORS[i];
                    if (isSel) {
                        trellis.pixels.setPixelColor(i, trellis.pixels.Color(c.r, c.g, c.b));
                    } else {
                        trellis.pixels.setPixelColor(i, trellis.pixels.Color(c.r / 4, c.g / 4, c.b / 4));
                    }
                } else { // Row 2 & 3: Track mutes
                    int trk = i - 8;
                    bool isMuted = app.brain.tracks[trk].muted;
                    if (isMuted) {
                        trellis.pixels.setPixelColor(i, trellis.pixels.Color(220, 30, 30));
                    } else {
                        trellis.pixels.setPixelColor(i, trellis.pixels.Color(20, 140, 60));
                    }
                }
            } else if (app.currentView == VIEW_SOUND_EDIT) {
                if (i < 8) { // Select Track
                    bool isSel = (app.brain.selectedTrack == i);
                    NeoRGB c = NEO_TRACK_COLORS[i];
                    if (isSel) {
                        trellis.pixels.setPixelColor(i, trellis.pixels.Color(c.r, c.g, c.b));
                    } else {
                        trellis.pixels.setPixelColor(i, trellis.pixels.Color(c.r / 5, c.g / 5, c.b / 5));
                    }
                } else if (i == 8 || i == 12) { // Sample - / + (A / Z)
                    trellis.pixels.setPixelColor(i, trellis.pixels.Color(0, 180, 220));
                } else if (i == 9 || i == 13) { // Pitch - / + (S / X)
                    trellis.pixels.setPixelColor(i, trellis.pixels.Color(255, 180, 0));
                } else if (i == 10 || i == 14) { // Volume - / + (D / C)
                    trellis.pixels.setPixelColor(i, trellis.pixels.Color(0, 240, 140));
                } else if (i == 11) { // Trigger (F)
                    trellis.pixels.setPixelColor(i, trellis.pixels.Color(0, 255, 100));
                } else if (i == 15) { // Mute (V)
                    bool isMuted = app.brain.tracks[app.brain.selectedTrack].muted;
                    trellis.pixels.setPixelColor(i, isMuted ? trellis.pixels.Color(220, 40, 40) : trellis.pixels.Color(60, 70, 85));
                }
            } else if (app.currentView == VIEW_CLIPS) {
                int selTrk = app.brain.selectedTrack;
                if (i < 8) { // Select Track
                    bool isSel = (selTrk == i);
                    NeoRGB c = NEO_TRACK_COLORS[i];
                    if (isSel) trellis.pixels.setPixelColor(i, trellis.pixels.Color(c.r, c.g, c.b));
                    else trellis.pixels.setPixelColor(i, trellis.pixels.Color(c.r / 4, c.g / 4, c.b / 4));
                } else { // Select Clip 1..8 for selected track
                    int clipIdx = i - 8;
                    bool isActiveClip = (app.brain.tracks[selTrk].activeClip == clipIdx);
                    bool isCreated = app.brain.tracks[selTrk].clips[clipIdx].isCreated;
                    NeoRGB c = NEO_TRACK_COLORS[selTrk];

                    if (isActiveClip) {
                        trellis.pixels.setPixelColor(i, trellis.pixels.Color(255, 255, 255));
                    } else if (isCreated) {
                        trellis.pixels.setPixelColor(i, trellis.pixels.Color(c.r / 2, c.g / 2, c.b / 2));
                    } else {
                        trellis.pixels.setPixelColor(i, trellis.pixels.Color(4, 6, 10));
                    }
                }
            } else if (app.currentView == VIEW_PROJECTS) {
                bool isLoaded = (app.currentProject == i);
                bool isOccupied = app.projects[i].isOccupied;
                bool isCopySrc = (app.copyState != COPY_IDLE && app.copySourcePad == i);
                bool isCopyDst = (app.copyState == COPY_CONFIRM_OVERWRITE && app.copyTargetPad == i);

                if (isCopyDst) {
                    trellis.pixels.setPixelColor(i, trellis.pixels.Color(255, 40, 40));
                } else if (isCopySrc) {
                    trellis.pixels.setPixelColor(i, trellis.pixels.Color(240, 160, 0));
                } else if (isLoaded) {
                    trellis.pixels.setPixelColor(i, trellis.pixels.Color(0, 220, 255));
                } else if (isOccupied) {
                    trellis.pixels.setPixelColor(i, trellis.pixels.Color(30, 140, 70));
                } else {
                    trellis.pixels.setPixelColor(i, trellis.pixels.Color(4, 6, 10));
                }
            } else if (app.currentView == VIEW_GLOBAL) {
                if (app.globalMenuMode) {
                    if (i == 0) { // Pad 0: TRIG RUN toggle
                        if (app.autoTriggerOnSelect) trellis.pixels.setPixelColor(i, trellis.pixels.Color(0, 180, 220));
                        else trellis.pixels.setPixelColor(i, trellis.pixels.Color(70, 80, 95));
                    } else if (i == 1) { // Pad 1: Brightness cycle button
                        trellis.pixels.setPixelColor(i, trellis.pixels.Color(240, 160, 0));
                    } else if (i >= 2 && i < 8) { // Pads 2..7: Empty
                        trellis.pixels.setPixelColor(i, trellis.pixels.Color(0, 0, 0));
                    }
                } else {
                    if (i < 8) { // Select Track 1..8
                        bool isSel = (app.brain.selectedTrack == i);
                        NeoRGB c = NEO_TRACK_COLORS[i];
                        if (isSel) trellis.pixels.setPixelColor(i, trellis.pixels.Color(c.r, c.g, c.b));
                        else trellis.pixels.setPixelColor(i, trellis.pixels.Color(c.r / 4, c.g / 4, c.b / 4));
                    }
                }

                if (i == 8) { // 'A': Menu Pad
                    if (app.globalMenuMode) trellis.pixels.setPixelColor(i, trellis.pixels.Color(255, 255, 255));
                    else trellis.pixels.setPixelColor(i, trellis.pixels.Color(80, 80, 80));
                } else if (i == 9 || i == 13) { // Empty
                    trellis.pixels.setPixelColor(i, trellis.pixels.Color(0, 0, 0));
                } else if (i == 10 || i == 14) { // BPM - / + (D / C)
                    trellis.pixels.setPixelColor(i, trellis.pixels.Color(230, 150, 0));
                } else if (i == 11 || i == 15) { // VOL - / + (F / V)
                    trellis.pixels.setPixelColor(i, trellis.pixels.Color(0, 180, 220));
                } else if (i == 12) { // Play/Pause (Z)
                    if (app.brain.isPlaying) trellis.pixels.setPixelColor(i, trellis.pixels.Color(40, 180, 80));
                    else trellis.pixels.setPixelColor(i, trellis.pixels.Color(200, 50, 50));
                }
            }
        }
        trellis.pixels.show();
    }
};
