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
    if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
        g_neoAppPtr->handlePadPress(padIdx, true);
    }
    return 0;
}

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
                trellis.registerCallback(i, neoTrellisCallback);
            }
        }
    }

    void update(ZicApp& app)
    {
        if (!initialized) return;
        trellis.read();

        // Update LED feedback on NeoTrellis matrix
        for (int i = 0; i < 16; i++) {
            if (app.currentView == VIEW_STEP_EDIT) {
                int trk = app.brain.selectedTrack;
                bool active = app.brain.tracks[trk].steps[i].active;
                bool isCurrent = (app.brain.currentStep == i && app.brain.isPlaying);

                if (isCurrent) {
                    trellis.pixels.setPixelColor(i, trellis.pixels.Color(255, 255, 255));
                } else if (active) {
                    trellis.pixels.setPixelColor(i, trellis.pixels.Color(0, 180, 220));
                } else {
                    trellis.pixels.setPixelColor(i, trellis.pixels.Color(5, 10, 15));
                }
            } else {
                if (i < 4) { // Row 0: Track select
                    bool isSel = (app.brain.selectedTrack == i);
                    trellis.pixels.setPixelColor(i, isSel ? trellis.pixels.Color(0, 220, 255) : trellis.pixels.Color(20, 40, 60));
                } else if (i >= 4 && i < 8) { // Row 1: Track Mute
                    bool isMuted = app.brain.tracks[i - 4].muted;
                    trellis.pixels.setPixelColor(i, isMuted ? trellis.pixels.Color(220, 30, 30) : trellis.pixels.Color(30, 180, 70));
                } else {
                    trellis.pixels.setPixelColor(i, trellis.pixels.Color(10, 15, 25));
                }
            }
        }
        trellis.pixels.show();
    }
};
