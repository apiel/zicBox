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

        int deltaX = x - touchDownX;
        int deltaY = y - touchDownY;

        // 1. Detect Vertical Slide (Track Change)
        if (std::abs(deltaY) > 25 && std::abs(deltaY) > std::abs(deltaX)) {
            if (deltaY < 0) { // Slide Up -> Previous Track
                app.brain.selectedTrack = (app.brain.selectedTrack + SequenceBrain::NUM_TRACKS - 1) % SequenceBrain::NUM_TRACKS;
            } else { // Slide Down -> Next Track
                app.brain.selectedTrack = (app.brain.selectedTrack + 1) % SequenceBrain::NUM_TRACKS;
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
        const char* viewNames[NUM_VIEWS] = { "OVERVIEW (8-TRK)", "STEP SEQUENCER", "SAMPLE EDITOR", "GLOBAL (4x4 MATRIX)" };
        snprintf(topBuf, sizeof(topBuf), "%s", viewNames[(int)app.currentView]);
        d.text({ 8, 4 }, topBuf, 12, textOpt(makeColor(0, 220, 255, 255)));

        snprintf(topBuf, sizeof(topBuf), "%d BPM  %s", (int)app.brain.bpm, app.brain.isPlaying ? "RUN" : "STOP");
        d.text({ 140, 4 }, topBuf, 12, textOpt(app.brain.isPlaying ? makeColor(0, 255, 128, 255) : makeColor(255, 100, 100, 255)));

        // 4 View Indicator Dots
        for (int i = 0; i < NUM_VIEWS; ++i) {
            Color dotCol = ((int)app.currentView == i) ? makeColor(0, 220, 255, 255) : makeColor(70, 80, 90, 255);
            d.filledCircle({ 255 + i * 16, 10 }, 4, drawOpt(dotCol));
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
    std::string trimName(const std::string& str, size_t maxLen = 14)
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
            d.textCentered({ tx + 18, 24 }, app.brain.tracks[t].name, 8, textOpt(textCol));
        }

        int trk = app.brain.selectedTrack;
        DrumTrack& track = app.brain.tracks[trk];
        Color activeColor = TRACK_COLORS[trk];

        int padW = 70;
        int padH = 27;
        int startX = 12;
        int startY = 40;

        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                int stepIdx = row * 4 + col;
                int padX = startX + col * (padW + 6);
                int padY = startY + row * (padH + 4);

                bool active = track.steps[stepIdx].active;
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
                d.textCentered({ padX + padW / 2, padY + 8 }, numBuf, 12, textOpt(textCol));

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
            d.textCentered({ tx + 18, 24 }, app.brain.tracks[t].name, 8, textOpt(textCol));
        }

        int trk = app.brain.selectedTrack;
        SampleTrack& sTrack = app.sampleTracks[trk];
        Color trkCol = TRACK_COLORS[trk];

        // 1. Sample Control
        d.filledRect({ 10, 42 }, { 200, 28 }, 4, drawOpt(makeColor(24, 30, 40, 255)));
        d.filledRect({ 10, 42 }, { 5, 28 }, 2, drawOpt(trkCol));
        d.text({ 20, 50 }, "SAMPLE", 8, textOpt(makeColor(180, 190, 200, 255)));

        std::string sampleDisplayName = trimName(g_presetSamples[sTrack.sampleIdx].name, 14);
        d.textRight({ 205, 50 }, sampleDisplayName, 12, textOpt(trkCol));

        d.filledRect({ 218, 42 }, { 44, 28 }, 4, drawOpt(makeColor(45, 55, 70, 255)));
        d.textCentered({ 240, 50 }, "A:-", 12, textOpt(makeColor(255, 255, 255, 255)));

        d.filledRect({ 268, 42 }, { 44, 28 }, 4, drawOpt(makeColor(45, 55, 70, 255)));
        d.textCentered({ 290, 50 }, "Z:+", 12, textOpt(makeColor(255, 255, 255, 255)));

        // 2. Pitch Control
        d.filledRect({ 10, 74 }, { 200, 28 }, 4, drawOpt(makeColor(24, 30, 40, 255)));
        d.text({ 20, 82 }, "PITCH", 8, textOpt(makeColor(180, 190, 200, 255)));
        char pBuf[32];
        snprintf(pBuf, sizeof(pBuf), "%+.0f ST", sTrack.pitch);
        d.textRight({ 205, 82 }, pBuf, 12, textOpt(makeColor(255, 208, 0, 255)));

        d.filledRect({ 218, 74 }, { 44, 28 }, 4, drawOpt(makeColor(45, 55, 70, 255)));
        d.textCentered({ 240, 82 }, "S:-", 12, textOpt(makeColor(255, 255, 255, 255)));

        d.filledRect({ 268, 74 }, { 44, 28 }, 4, drawOpt(makeColor(45, 55, 70, 255)));
        d.textCentered({ 290, 82 }, "X:+", 12, textOpt(makeColor(255, 255, 255, 255)));

        // 3. Volume Control (0% to 200% Gain)
        d.filledRect({ 10, 106 }, { 200, 28 }, 4, drawOpt(makeColor(24, 30, 40, 255)));
        d.text({ 20, 114 }, "VOLUME", 8, textOpt(makeColor(180, 190, 200, 255)));
        char vBuf[32];
        snprintf(vBuf, sizeof(vBuf), "%d%%", (int)(sTrack.volume * 100.0f));
        d.textRight({ 205, 114 }, vBuf, 12, textOpt(makeColor(0, 255, 160, 255)));

        d.filledRect({ 218, 106 }, { 44, 28 }, 4, drawOpt(makeColor(45, 55, 70, 255)));
        d.textCentered({ 240, 114 }, "D:-", 12, textOpt(makeColor(255, 255, 255, 255)));

        d.filledRect({ 268, 106 }, { 44, 28 }, 4, drawOpt(makeColor(45, 55, 70, 255)));
        d.textCentered({ 290, 114 }, "C:+", 12, textOpt(makeColor(255, 255, 255, 255)));

        // 4. Quick Actions (F: Trigger, V: Mute)
        d.filledRect({ 10, 138 }, { 146, 26 }, 4, drawOpt(makeColor(30, 140, 70, 255)));
        d.textCentered({ 83, 145 }, "F: TRIG SAMPLE", 8, textOpt(makeColor(255, 255, 255, 255)));

        Color muteBg = sTrack.muted ? makeColor(180, 40, 40, 255) : makeColor(50, 60, 75, 255);
        d.filledRect({ 164, 138 }, { 148, 26 }, 4, drawOpt(muteBg));
        d.textCentered({ 238, 145 }, sTrack.muted ? "V: UNMUTE TRACK" : "V: MUTE TRACK", 8, textOpt(makeColor(255, 255, 255, 255)));
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
            const char* keyLabel;
            const char* actionName;
            Color cellColor;
            bool isSelected;
        };

        GridCell cells[16];

        // Row 0
        cells[0] = { "1", app.brain.isPlaying ? "PAUSE" : "PLAY", app.brain.isPlaying ? makeColor(40, 180, 80, 255) : makeColor(200, 50, 50, 255), false };
        cells[1] = { "2", "BPM -5", makeColor(220, 130, 0, 255), false };
        cells[2] = { "3", "BPM +5", makeColor(240, 170, 0, 255), false };
        cells[3] = { "4", "GEN PAT", makeColor(0, 170, 220, 255), false };

        // Row 1 (Tracks 1..4)
        for (int i = 0; i < 4; ++i) {
            bool isSel = (app.brain.selectedTrack == i);
            cells[4 + i] = {
                (i == 0 ? "Q" : i == 1 ? "W" : i == 2 ? "E" : "R"),
                app.brain.tracks[i].name,
                TRACK_COLORS[i],
                isSel
            };
        }

        // Row 2 (Tracks 5..8)
        for (int i = 0; i < 4; ++i) {
            int trk = 4 + i;
            bool isSel = (app.brain.selectedTrack == trk);
            cells[8 + i] = {
                (i == 0 ? "A" : i == 1 ? "S" : i == 2 ? "D" : "F"),
                app.brain.tracks[trk].name,
                TRACK_COLORS[trk],
                isSel
            };
        }

        // Row 3
        cells[12] = { "Z", "PREV VIEW", makeColor(140, 60, 200, 255), false };
        cells[13] = { "X", "NEXT VIEW", makeColor(170, 80, 230, 255), false };

        bool isMuted = app.brain.tracks[app.brain.selectedTrack].muted;
        cells[14] = { "C", isMuted ? "UNMUTE" : "MUTE", isMuted ? makeColor(220, 40, 40, 255) : makeColor(100, 110, 125, 255), false };
        cells[15] = { "V", "TRIG SEL", TRACK_COLORS[app.brain.selectedTrack], false };

        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                int idx = r * 4 + c;
                int cx = startX + c * (cellW + gapX);
                int cy = startY + r * (cellH + gapY);
                GridCell& cell = cells[idx];

                d.filledRect({ cx, cy }, { cellW, cellH }, 3, drawOpt(cell.cellColor));

                if (cell.isSelected) {
                    d.rect({ cx - 1, cy - 1 }, { cellW + 2, cellH + 2 }, 3, drawOpt(makeColor(255, 255, 255, 255)));
                }

                // Key Label (Top-Left badge)
                d.text({ cx + 4, cy + 3 }, cell.keyLabel, 8, textOpt(makeColor(0, 0, 0, 255)));

                // Action Name (Center)
                d.textCentered({ cx + cellW / 2, cy + 12 }, cell.actionName, 8, textOpt(makeColor(0, 0, 0, 255)));
            }
        }
    }
};
