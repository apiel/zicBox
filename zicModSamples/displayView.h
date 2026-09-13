#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

#ifndef SCREEN_BUFFER_ROWS
#define SCREEN_BUFFER_ROWS 170
#endif
#ifndef SCREEN_BUFFER_COLS
#define SCREEN_BUFFER_COLS 320
#endif

#include "draw/draw.h"
#include "zicApp.h"

inline Color makeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
{
    Color c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
    return c;
}

inline DrawOptions drawOpt(Color color, int thickness = 1)
{
    DrawOptions opt;
    opt.color = color;
    opt.thickness = thickness;
    return opt;
}

inline DrawTextOptions textOpt(Color color, void* font = nullptr)
{
    DrawTextOptions opt;
    opt.color = color;
    opt.font = font;
    return opt;
}

// 8 Distinct Track Colors
static const Color TRACK_COLORS[8] = {
    { 0, 210, 255, 255 },   // Trk 0: Electric Cyan
    { 255, 0, 127, 255 },   // Trk 1: Hot Pink
    { 255, 208, 0, 255 },   // Trk 2: Amber Yellow
    { 0, 255, 102, 255 },   // Trk 3: Neon Green
    { 255, 102, 0, 255 },   // Trk 4: Vibrant Orange
    { 153, 51, 255, 255 },  // Trk 5: Violet Purple
    { 255, 51, 51, 255 },   // Trk 6: Bright Red
    { 0, 255, 204, 255 }    // Trk 7: Bright Teal
};

class DisplayView {
public:
    static constexpr int SCREEN_W = 320;
    static constexpr int SCREEN_H = 170;

    int touchDownX = 0;
    int touchDownY = 0;
    bool isTouching = false;

    void handleTouchDown(int x, int y)
    {
        touchDownX = x;
        touchDownY = y;
        isTouching = true;
    }

    void handleTouchMove(int x, int y)
    {
        // Touch drag tracking
    }

    void handleTouchUp(int x, int y, ZicApp& app)
    {
        if (!isTouching) return;
        isTouching = false;

        // If Probability Sub-Menu is open, tap selects preset
        if (app.showProbSubMenu) {
            int pW = 68;
            int pH = 22;
            int pStartX = 14;
            int pStartY = 56;

            for (int r = 0; r < 4; ++r) {
                for (int c = 0; c < 4; ++c) {
                    int pIdx = r * 4 + c;
                    int px = pStartX + c * (pW + 6);
                    int py = pStartY + r * (pH + 4);
                    if (x >= px && x <= px + pW && y >= py && y <= py + pH) {
                        app.handlePadEvent(pIdx, true);
                        return;
                    }
                }
            }
            app.showProbSubMenu = false;
            return;
        }

        int deltaX = x - touchDownX;
        int deltaY = y - touchDownY;

        // 1. Detect Vertical Slide (Track Change)
        if (std::abs(deltaY) > 25 && std::abs(deltaY) > std::abs(deltaX)) {
            if (deltaY < 0) { // Slide Up -> Previous Track
                app.selectTrack(app.brain.selectedTrack - 1);
            } else { // Slide Down -> Next Track
                app.selectTrack(app.brain.selectedTrack + 1);
            }
            return;
        }

        // 2. Touch Navigation: Left side = Prev View, Right side = Next View
        if (x <= SCREEN_W / 2 || deltaX < -25) {
            app.prevView();
        } else {
            app.nextView();
        }
    }

    void render(Draw& d, ZicApp& app)
    {
        d.clear();

        // Top Navigation Bar
        d.filledRect({ 0, 0 }, { SCREEN_W, 20 }, drawOpt(makeColor(20, 24, 32, 255)));

        char topBuf[64];
        const char* viewNames[NUM_VIEWS] = { "OVERVIEW", "STEP SEQ", "SOUND EDIT", "GLOBAL" };
        snprintf(topBuf, sizeof(topBuf), "%s", viewNames[(int)app.currentView]);
        d.text({ 6, 6 }, topBuf, 8, textOpt(makeColor(0, 220, 255, 255)));

        snprintf(topBuf, sizeof(topBuf), "%dBPM  VOL:%d%%  %s", (int)app.brain.bpm, (int)(app.masterVolume * 100.0f), app.brain.isPlaying ? "RUN" : "STOP");
        d.text({ 110, 6 }, topBuf, 8, textOpt(app.brain.isPlaying ? makeColor(0, 255, 128, 255) : makeColor(255, 100, 100, 255)));

        // 4 View Indicator Dots
        for (int i = 0; i < NUM_VIEWS; ++i) {
            Color dotCol = ((int)app.currentView == i) ? makeColor(0, 220, 255, 255) : makeColor(60, 70, 80, 255);
            d.filledCircle({ 265 + i * 13, 10 }, 3, drawOpt(dotCol));
        }

        if (app.currentView == VIEW_OVERVIEW) {
            renderOverview(d, app);
        } else if (app.currentView == VIEW_STEP_EDIT) {
            renderStepEdit(d, app);
        } else if (app.currentView == VIEW_SOUND_EDIT) {
            renderSoundEdit(d, app);
        } else if (app.currentView == VIEW_GLOBAL) {
            renderGlobal(d, app);
        }
    }

private:
    std::string trimName(const std::string& str, size_t maxLen = 24)
    {
        if (str.length() <= maxLen) return str;
        return str.substr(0, maxLen - 3) + "...";
    }

    void renderOverview(Draw& d, ZicApp& app)
    {
        for (int t = 0; t < SequenceBrain::NUM_TRACKS; ++t) {
            int rowY = 22 + t * 18;
            bool isSel = (app.brain.selectedTrack == t);
            bool isMuted = app.brain.tracks[t].muted;

            Color trkCol = TRACK_COLORS[t];
            Color bgCol = isSel ? makeColor(40, 48, 62, 255) : makeColor(16, 20, 26, 255);
            d.filledRect({ 4, rowY }, { 312, 17 }, 2, drawOpt(bgCol));

            if (isSel) {
                d.rect({ 4, rowY }, { 312, 17 }, 2, drawOpt(trkCol));
            }

            // Track Color Indicator Pill
            d.filledRect({ 6, rowY + 3 }, { 5, 11 }, 1, drawOpt(trkCol));

            // Track Name
            Color nameCol = isSel ? makeColor(255, 255, 255, 255) : makeColor(180, 190, 200, 255);
            d.text({ 15, rowY + 3 }, app.brain.tracks[t].name, 8, textOpt(nameCol));

            // Mute Status Badge
            Color muteBg = isMuted ? makeColor(180, 40, 40, 255) : makeColor(25, 130, 65, 255);
            d.filledRect({ 65, rowY + 2 }, { 45, 13 }, 2, drawOpt(muteBg));
            d.textCentered({ 87, rowY + 3 }, isMuted ? "MUTED" : "ON", 8, textOpt(makeColor(255, 255, 255, 255)));

            // 16-Step Preview Grid
            int stepStartX = 120;
            for (int s = 0; s < SequenceBrain::NUM_STEPS; ++s) {
                int sx = stepStartX + s * 12;
                bool active = app.brain.tracks[t].steps[s].active;
                bool isCurrent = (app.brain.currentStep == s && app.brain.isPlaying);

                Color stepCol;
                if (active) {
                    stepCol = isMuted ? makeColor(110, 110, 50, 255) : trkCol;
                } else {
                    stepCol = makeColor(35, 40, 50, 255);
                }

                d.filledRect({ sx, rowY + 3 }, { 10, 11 }, 1, drawOpt(stepCol));

                if (isCurrent) {
                    d.rect({ sx - 1, rowY + 2 }, { 12, 13 }, drawOpt(makeColor(255, 255, 255, 255)));
                }
            }
        }
    }

    void renderStepEdit(Draw& d, ZicApp& app)
    {
        // 8 Track selection tabs (y: 22..36)
        for (int t = 0; t < SequenceBrain::NUM_TRACKS; ++t) {
            int tx = 4 + t * 39;
            bool isSel = (app.brain.selectedTrack == t);
            Color trkCol = TRACK_COLORS[t];
            Color tabBg = isSel ? trkCol : makeColor(28, 34, 44, 255);
            d.filledRect({ tx, 22 }, { 36, 15 }, 2, drawOpt(tabBg));

            Color textCol = isSel ? makeColor(0, 0, 0, 255) : makeColor(220, 230, 240, 255);
            char tabLabel[8];
            snprintf(tabLabel, sizeof(tabLabel), "T%d", t + 1);
            d.textCentered({ tx + 18, 24 }, tabLabel, 8, textOpt(textCol));
        }

        int trk = app.brain.selectedTrack;
        DrumTrack& track = app.brain.tracks[trk];
        Color activeColor = TRACK_COLORS[trk];

        int padW = 70;
        int padH = 27;
        int startX = 12;
        int startY = 40;

        // Render Probability Sub-Menu Overlay if open
        if (app.showProbSubMenu) {
            d.filledRect({ 8, 38 }, { 304, 128 }, 4, drawOpt(makeColor(18, 22, 30, 245)));
            d.rect({ 8, 38 }, { 304, 128 }, 4, drawOpt(makeColor(0, 200, 255, 255)));

            char subTitle[64];
            snprintf(subTitle, sizeof(subTitle), "STEP %d PROBABILITY SELECTION", app.probEditingStep + 1);
            d.textCentered({ SCREEN_W / 2, 43 }, subTitle, 8, textOpt(makeColor(0, 220, 255, 255)));

            int pW = 68;
            int pH = 22;
            int pStartX = 14;
            int pStartY = 56;

            for (int r = 0; r < 4; ++r) {
                for (int c = 0; c < 4; ++c) {
                    int pIdx = r * 4 + c;
                    int px = pStartX + c * (pW + 6);
                    int py = pStartY + r * (pH + 4);
                    uint8_t presetVal = PROBABILITY_PRESETS[pIdx];
                    uint8_t currStepProb = track.steps[app.probEditingStep].probability;

                    Color bgCol;
                    if (presetVal == 100) bgCol = makeColor(0, 180, 90, 255);
                    else if (presetVal == 0) bgCol = makeColor(150, 40, 40, 255);
                    else bgCol = makeColor(0, 140, 220, 255);

                    d.filledRect({ px, py }, { pW, pH }, 3, drawOpt(bgCol));

                    if (presetVal == currStepProb) {
                        d.rect({ px - 1, py - 1 }, { pW + 2, pH + 2 }, 3, drawOpt(makeColor(255, 255, 255, 255)));
                    }

                    char pStr[16];
                    snprintf(pStr, sizeof(pStr), "%d%%", presetVal);
                    Color textCol = (presetVal == currStepProb) ? makeColor(255, 255, 255, 255) : makeColor(220, 230, 240, 255);
                    d.textCentered({ px + pW / 2, py + 7 }, pStr, 8, textOpt(textCol));
                }
            }
            return;
        }

        // Standard 16 Step Grid
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                int stepIdx = row * 4 + col;
                int padX = startX + col * (padW + 6);
                int padY = startY + row * (padH + 4);

                bool active = track.steps[stepIdx].active;
                uint8_t prob = track.steps[stepIdx].probability;
                bool isPlayhead = (app.brain.currentStep == stepIdx && app.brain.isPlaying);

                Color padBg;
                if (active) {
                    padBg = track.muted ? makeColor(120, 80, 25, 255) : activeColor;
                } else {
                    padBg = makeColor(28, 32, 42, 255);
                }

                d.filledRect({ padX, padY }, { padW, padH }, 4, drawOpt(padBg));

                char numBuf[8];
                snprintf(numBuf, sizeof(numBuf), "%d", stepIdx + 1);
                Color textCol = (active && !track.muted) ? makeColor(0, 0, 0, 255) : makeColor(160, 170, 180, 255);

                if (active && prob < 100) {
                    d.text({ padX + 6, padY + 8 }, numBuf, 8, textOpt(textCol));
                    char probBuf[8];
                    snprintf(probBuf, sizeof(probBuf), "%d%%", prob);
                    // Dark high-contrast pill badge
                    d.filledRect({ padX + padW - 34, padY + 4 }, { 30, 14 }, 2, drawOpt(makeColor(12, 16, 24, 230)));
                    d.textRight({ padX + padW - 6, padY + 7 }, probBuf, 8, textOpt(makeColor(255, 255, 255, 255)));
                } else {
                    d.textCentered({ padX + padW / 2, padY + 8 }, numBuf, 12, textOpt(textCol));
                }

                if (isPlayhead) {
                    d.rect({ padX - 1, padY - 1 }, { padW + 2, padH + 2 }, 4, drawOpt(makeColor(255, 255, 255, 255)));
                }
            }
        }
    }

    void renderSoundEdit(Draw& d, ZicApp& app)
    {
        // 8 Track selection tabs (y: 22..36)
        for (int t = 0; t < SequenceBrain::NUM_TRACKS; ++t) {
            int tx = 4 + t * 39;
            bool isSel = (app.brain.selectedTrack == t);
            Color trkCol = TRACK_COLORS[t];
            Color tabBg = isSel ? trkCol : makeColor(28, 34, 44, 255);
            d.filledRect({ tx, 22 }, { 36, 15 }, 2, drawOpt(tabBg));

            Color textCol = isSel ? makeColor(0, 0, 0, 255) : makeColor(220, 230, 240, 255);
            char tabLabel[8];
            snprintf(tabLabel, sizeof(tabLabel), "T%d", t + 1);
            d.textCentered({ tx + 18, 24 }, tabLabel, 8, textOpt(textCol));
        }

        int trk = app.brain.selectedTrack;
        SampleTrack& sTrack = app.sampleTracks[trk];
        Color trkCol = TRACK_COLORS[trk];

        // 1. Sample Control
        Color samplePadCol = makeColor(0, 180, 220, 255); // Pad 8 / 12 Cyan
        d.filledRect({ 10, 42 }, { 200, 28 }, 4, drawOpt(makeColor(24, 30, 40, 255)));
        d.filledRect({ 10, 42 }, { 5, 28 }, 2, drawOpt(trkCol));
        d.text({ 20, 52 }, "SAMPLE", 8, textOpt(makeColor(180, 190, 200, 255)));

        std::string sampleDisplayName = trimName(g_presetSamples[sTrack.sampleIdx].name, 24);
        d.textRight({ 205, 52 }, sampleDisplayName, 8, textOpt(trkCol));

        d.filledRect({ 218, 42 }, { 44, 28 }, 4, drawOpt(samplePadCol));
        d.textCentered({ 240, 50 }, "-", 12, textOpt(makeColor(0, 0, 0, 255)));

        d.filledRect({ 268, 42 }, { 44, 28 }, 4, drawOpt(samplePadCol));
        d.textCentered({ 290, 50 }, "+", 12, textOpt(makeColor(0, 0, 0, 255)));

        // 2. Pitch Control
        Color pitchPadCol = makeColor(255, 180, 0, 255); // Pad 9 / 13 Amber
        d.filledRect({ 10, 74 }, { 200, 28 }, 4, drawOpt(makeColor(24, 30, 40, 255)));
        d.text({ 20, 84 }, "PITCH", 8, textOpt(makeColor(180, 190, 200, 255)));
        char pBuf[32];
        snprintf(pBuf, sizeof(pBuf), "%+.0f ST", sTrack.pitch);
        d.textRight({ 205, 84 }, pBuf, 8, textOpt(pitchPadCol));

        d.filledRect({ 218, 74 }, { 44, 28 }, 4, drawOpt(pitchPadCol));
        d.textCentered({ 240, 82 }, "-", 12, textOpt(makeColor(0, 0, 0, 255)));

        d.filledRect({ 268, 74 }, { 44, 28 }, 4, drawOpt(pitchPadCol));
        d.textCentered({ 290, 82 }, "+", 12, textOpt(makeColor(0, 0, 0, 255)));

        // 3. Volume Control (0% to 200% Gain)
        Color volPadCol = makeColor(0, 240, 140, 255); // Pad 10 / 14 Green
        d.filledRect({ 10, 106 }, { 200, 28 }, 4, drawOpt(makeColor(24, 30, 40, 255)));
        d.text({ 20, 116 }, "VOLUME", 8, textOpt(makeColor(180, 190, 200, 255)));
        char vBuf[32];
        snprintf(vBuf, sizeof(vBuf), "%d%%", (int)(sTrack.volume * 100.0f));
        d.textRight({ 205, 116 }, vBuf, 8, textOpt(volPadCol));

        d.filledRect({ 218, 106 }, { 44, 28 }, 4, drawOpt(volPadCol));
        d.textCentered({ 240, 114 }, "-", 12, textOpt(makeColor(0, 0, 0, 255)));

        d.filledRect({ 268, 106 }, { 44, 28 }, 4, drawOpt(volPadCol));
        d.textCentered({ 290, 114 }, "+", 12, textOpt(makeColor(0, 0, 0, 255)));

        // 4. Quick Actions (Trigger, Mute)
        d.filledRect({ 10, 138 }, { 146, 26 }, 4, drawOpt(makeColor(0, 255, 100, 255)));
        d.textCentered({ 83, 145 }, "TRIG SAMPLE", 8, textOpt(makeColor(0, 0, 0, 255)));

        Color muteBg = sTrack.muted ? makeColor(220, 40, 40, 255) : makeColor(60, 70, 85, 255);
        d.filledRect({ 164, 138 }, { 148, 26 }, 4, drawOpt(muteBg));
        d.textCentered({ 238, 145 }, sTrack.muted ? "UNMUTE TRACK" : "MUTE TRACK", 8, textOpt(makeColor(255, 255, 255, 255)));
    }

    void renderGlobal(Draw& d, ZicApp& app)
    {
        // 4x4 Grid of 16 NeoTrellis Cells
        int cellW = 73;
        int cellH = 32;
        int startX = 6;
        int startY = 24;
        int gapX = 5;
        int gapY = 4;

        struct GridCell {
            const char* actionName;
            Color cellColor;
            bool isSelected;
            bool isEmpty;
        };

        GridCell cells[16];

        // Row 0 & Row 1: Tracks 1 to 8 (Pads 0..7)
        for (int i = 0; i < 8; ++i) {
            bool isSel = (app.brain.selectedTrack == i);
            cells[i] = {
                app.brain.tracks[i].name,
                TRACK_COLORS[i],
                isSel,
                false
            };
        }

        // Row 2 (Pads 8..11)
        cells[8]  = { app.autoTriggerOnSelect ? "TRIG: RUN" : "TRIG: OFF", app.autoTriggerOnSelect ? makeColor(0, 180, 220, 255) : makeColor(70, 80, 95, 255), false, false }; // 'A': Toggle Trig on Select
        cells[9]  = { "", makeColor(22, 26, 34, 255), false, true };  // 'S': Empty
        cells[10] = { "BPM -5", makeColor(220, 130, 0, 255), false, false }; // 'D': BPM -5
        cells[11] = { "VOL -", makeColor(0, 180, 220, 255), false, false };  // 'F': Master VOL -

        // Row 3 (Pads 12..15)
        cells[12] = { app.brain.isPlaying ? "PAUSE" : "PLAY", app.brain.isPlaying ? makeColor(40, 180, 80, 255) : makeColor(200, 50, 50, 255), false, false }; // 'Z': Play/Pause
        cells[13] = { "", makeColor(22, 26, 34, 255), false, true };  // 'X': Empty
        cells[14] = { "BPM +5", makeColor(240, 170, 0, 255), false, false }; // 'C': BPM +5
        cells[15] = { "VOL +", makeColor(0, 180, 220, 255), false, false };  // 'V': Master VOL +

        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                int idx = r * 4 + c;
                int cx = startX + c * (cellW + gapX);
                int cy = startY + r * (cellH + gapY);
                GridCell& cell = cells[idx];

                if (cell.isEmpty) {
                    d.rect({ cx, cy }, { cellW, cellH }, 3, drawOpt(makeColor(35, 42, 54, 255)));
                } else {
                    d.filledRect({ cx, cy }, { cellW, cellH }, 3, drawOpt(cell.cellColor));

                    if (cell.isSelected) {
                        d.rect({ cx - 1, cy - 1 }, { cellW + 2, cellH + 2 }, 3, drawOpt(makeColor(255, 255, 255, 255)));
                    }

                    // Action Name (Centered in cell)
                    Color txtCol = (idx < 8) ? makeColor(0, 0, 0, 255) : makeColor(255, 255, 255, 255);
                    d.textCentered({ cx + cellW / 2, cy + 10 }, cell.actionName, 8, textOpt(txtCol));
                }
            }
        }
    }
};
