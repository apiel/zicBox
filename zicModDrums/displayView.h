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

inline DrawTextOptions textOpt(Color color)
{
    DrawTextOptions opt;
    opt.color = color;
    return opt;
}

class DisplayView {
public:
    static constexpr int SCREEN_W = 320;
    static constexpr int SCREEN_H = 170;

    // Touch Swipe Tracking State
    bool touchDown = false;
    int touchStartX = 0;
    int touchStartY = 0;
    int touchCurrentX = 0;
    int touchCurrentY = 0;

    void handleTouchDown(int x, int y)
    {
        touchDown = true;
        touchStartX = x;
        touchStartY = y;
        touchCurrentX = x;
        touchCurrentY = y;
    }

    void handleTouchMove(int x, int y)
    {
        if (touchDown) {
            touchCurrentX = x;
            touchCurrentY = y;
        }
    }

    void handleTouchUp(int x, int y, ZicApp& app)
    {
        if (!touchDown) return;
        touchDown = false;

        int dx = x - touchStartX;
        int dy = y - touchStartY;

        // Swipe threshold: 40 pixels horizontal
        if (std::abs(dx) > 40 && std::abs(dx) > std::abs(dy)) {
            if (dx < 0) {
                app.nextView();
            } else {
                app.prevView();
            }
            return;
        }

        // Tap gesture (dx, dy small)
        handleTap(touchStartX, touchStartY, app);
    }

    void handleTap(int x, int y, ZicApp& app)
    {
        // Top Bar View Dots: Tap dots to jump to view
        if (y < 22) {
            if (x > 240) {
                int dotIdx = (x - 240) / 20;
                if (dotIdx >= 0 && dotIdx < NUM_VIEWS) {
                    app.setView(dotIdx);
                    return;
                }
            }
        }

        if (app.currentView == VIEW_OVERVIEW) {
            // 4 Track rows
            for (int t = 0; t < SequenceBrain::NUM_TRACKS; ++t) {
                int rowY = 24 + t * 36;
                // Track name select (left side)
                if (x >= 4 && x <= 80 && y >= rowY && y <= rowY + 32) {
                    app.brain.selectedTrack = t;
                    app.activeParamIdx = 0;
                    return;
                }
                // Mute button (middle)
                if (x >= 84 && x <= 140 && y >= rowY && y <= rowY + 32) {
                    app.brain.toggleMute(t);
                    return;
                }
                // Step preview (right side) -> open Step View for this track
                if (x >= 144 && x <= 316 && y >= rowY && y <= rowY + 32) {
                    app.brain.selectedTrack = t;
                    app.activeParamIdx = 0;
                    app.currentView = VIEW_STEP_EDIT;
                    return;
                }
            }
        } else if (app.currentView == VIEW_STEP_EDIT) {
            // Track selector header pills (y: 22..36)
            if (y >= 22 && y <= 36) {
                for (int t = 0; t < 4; ++t) {
                    int tx = 4 + t * 78;
                    if (x >= tx && x <= tx + 74) {
                        app.brain.selectedTrack = t;
                        return;
                    }
                }
            }

            // 4x4 Grid Pads (x: 12..310, y: 40..165)
            int padW = 70;
            int padH = 27;
            int startX = 12;
            int startY = 40;

            for (int row = 0; row < 4; ++row) {
                for (int col = 0; col < 4; ++col) {
                    int padX = startX + col * (padW + 6);
                    int padY = startY + row * (padH + 4);
                    if (x >= padX && x <= padX + padW && y >= padY && y <= padY + padH) {
                        int stepIdx = row * 4 + col;
                        app.brain.toggleStep(app.brain.selectedTrack, stepIdx);
                        return;
                    }
                }
            }
        } else if (app.currentView == VIEW_SOUND_EDIT) {
            // Track selector tabs at top (y: 22..36)
            if (y >= 22 && y <= 36) {
                for (int t = 0; t < 4; ++t) {
                    int tx = 4 + t * 78;
                    if (x >= tx && x <= tx + 74) {
                        app.brain.selectedTrack = t;
                        app.activeParamIdx = 0;
                        return;
                    }
                }
            }

            // Page navigation buttons [<] and [>] (y: 38..52, x: 250..310)
            if (y >= 38 && y <= 52) {
                if (x >= 250 && x <= 278) {
                    app.selectPrevParam();
                    return;
                }
                if (x >= 282 && x <= 310) {
                    app.selectNextParam();
                    return;
                }
            }

            // 4 Parameter rows (y: 56..168)
            int pageOffset = (app.activeParamIdx / 4) * 4;
            for (int i = 0; i < 4; ++i) {
                int paramY = 56 + i * 28;
                if (x >= 10 && x <= 240 && y >= paramY && y <= paramY + 26) {
                    IEngine* eng = app.getSelectedEngine();
                    if (eng && (size_t)(pageOffset + i) < eng->getParamCount()) {
                        app.activeParamIdx = pageOffset + i;
                    }
                    return;
                }
                if (x > 240 && x <= 275 && y >= paramY && y <= paramY + 26) {
                    app.activeParamIdx = pageOffset + i;
                    app.adjustSelectedParam(-1.0f);
                    return;
                }
                if (x > 275 && x <= 310 && y >= paramY && y <= paramY + 26) {
                    app.activeParamIdx = pageOffset + i;
                    app.adjustSelectedParam(1.0f);
                    return;
                }
            }
        } else if (app.currentView == VIEW_GLOBAL) {
            // Play/Stop button (y: 38..74, x: 10..155)
            if (x >= 10 && x <= 155 && y >= 38 && y <= 74) {
                app.brain.isPlaying = !app.brain.isPlaying;
                return;
            }
            // Generate pattern (y: 38..74, x: 165..310)
            if (x >= 165 && x <= 310 && y >= 38 && y <= 74) {
                app.brain.generatePattern();
                return;
            }
            // BPM -5 / +5 (y: 84..120)
            if (y >= 84 && y <= 120) {
                if (x >= 10 && x <= 155) {
                    app.brain.setBpm(app.brain.bpm - 5.0f);
                    return;
                }
                if (x >= 165 && x <= 310) {
                    app.brain.setBpm(app.brain.bpm + 5.0f);
                    return;
                }
            }
        }
    }

    void render(Draw& d, ZicApp& app)
    {
        d.clear();

        // -------------------------------------------------------------
        // TOP NAVIGATION BAR (y: 0..20)
        // -------------------------------------------------------------
        d.filledRect({ 0, 0 }, { SCREEN_W, 20 }, drawOpt(makeColor(24, 28, 36, 255)));

        char topBuf[64];
        const char* viewNames[NUM_VIEWS] = { "OVERVIEW & MUTE", "STEP SEQUENCER", "SOUND EDITOR", "GLOBAL SETTINGS" };
        snprintf(topBuf, sizeof(topBuf), "%s", viewNames[(int)app.currentView]);
        d.text({ 8, 4 }, topBuf, 12, textOpt(makeColor(0, 220, 255, 255)));

        // BPM & Play state
        snprintf(topBuf, sizeof(topBuf), "%d BPM  %s", (int)app.brain.bpm, app.brain.isPlaying ? "RUN" : "STOP");
        d.text({ 140, 4 }, topBuf, 12, textOpt(app.brain.isPlaying ? makeColor(0, 255, 128, 255) : makeColor(255, 100, 100, 255)));

        // 4 View Indicator Dots (top right: x=255..300)
        for (int i = 0; i < NUM_VIEWS; ++i) {
            Color dotCol = ((int)app.currentView == i) ? makeColor(0, 220, 255, 255) : makeColor(80, 90, 100, 255);
            d.filledCircle({ 255 + i * 14, 10 }, 4, drawOpt(dotCol));
        }

        // -------------------------------------------------------------
        // VIEW RENDER ROUTER
        // -------------------------------------------------------------
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
    void renderOverview(Draw& d, ZicApp& app)
    {
        for (int t = 0; t < SequenceBrain::NUM_TRACKS; ++t) {
            int rowY = 24 + t * 36;
            bool isSel = (app.brain.selectedTrack == t);
            bool isMuted = app.brain.tracks[t].muted;

            // Row background
            Color bgCol = isSel ? makeColor(40, 50, 65, 255) : makeColor(18, 22, 28, 255);
            d.filledRect({ 4, rowY }, { 312, 32 }, 4, drawOpt(bgCol));

            // Selection indicator border
            if (isSel) {
                d.rect({ 4, rowY }, { 312, 32 }, 4, drawOpt(makeColor(0, 200, 255, 255)));
            }

            // Track Name
            Color nameCol = isSel ? makeColor(255, 255, 255, 255) : makeColor(180, 190, 200, 255);
            d.text({ 10, rowY + 9 }, app.brain.tracks[t].name, 12, textOpt(nameCol));

            // Mute Button
            Color muteBg = isMuted ? makeColor(180, 40, 40, 255) : makeColor(30, 140, 70, 255);
            d.filledRect({ 84, rowY + 4 }, { 56, 24 }, 3, drawOpt(muteBg));
            d.textCentered({ 112, rowY + 8 }, isMuted ? "MUTED" : "ON", 10, textOpt(makeColor(255, 255, 255, 255)));

            // Mini 16-Step Preview Grid
            int stepStartX = 148;
            for (int s = 0; s < SequenceBrain::NUM_STEPS; ++s) {
                int sx = stepStartX + s * 10;
                bool active = app.brain.tracks[t].steps[s].active;
                bool isCurrent = (app.brain.currentStep == s && app.brain.isPlaying);

                Color stepCol;
                if (active) {
                    stepCol = isMuted ? makeColor(120, 120, 60, 255) : makeColor(0, 220, 160, 255);
                } else {
                    stepCol = makeColor(40, 45, 55, 255);
                }

                d.filledRect({ sx, rowY + 8 }, { 8, 16 }, 1, drawOpt(stepCol));

                // Playhead indicator border
                if (isCurrent) {
                    d.rect({ sx - 1, rowY + 7 }, { 10, 18 }, drawOpt(makeColor(255, 255, 255, 255)));
                }
            }
        }
    }

    void renderStepEdit(Draw& d, ZicApp& app)
    {
        // Track selection tabs (y: 22..36)
        for (int t = 0; t < 4; ++t) {
            int tx = 4 + t * 78;
            bool isSel = (app.brain.selectedTrack == t);
            Color tabBg = isSel ? makeColor(0, 180, 220, 255) : makeColor(30, 36, 45, 255);
            d.filledRect({ tx, 22 }, { 74, 15 }, 3, drawOpt(tabBg));
            d.textCentered({ tx + 37, 24 }, app.brain.tracks[t].name, 10, textOpt(makeColor(255, 255, 255, 255)));
        }

        int trk = app.brain.selectedTrack;
        DrumTrack& track = app.brain.tracks[trk];

        // 4x4 Grid of Steps (Row 0: 0..3, Row 1: 4..7, Row 2: 8..11, Row 3: 12..15)
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
                    padBg = track.muted ? makeColor(140, 100, 30, 255) : makeColor(0, 150, 200, 255);
                } else {
                    padBg = makeColor(30, 35, 45, 255);
                }

                d.filledRect({ padX, padY }, { padW, padH }, 4, drawOpt(padBg));

                // Step Number Label
                char numBuf[8];
                snprintf(numBuf, sizeof(numBuf), "%d", stepIdx + 1);
                Color textCol = active ? makeColor(255, 255, 255, 255) : makeColor(120, 130, 140, 255);
                d.textCentered({ padX + padW / 2, padY + 8 }, numBuf, 12, textOpt(textCol));

                // Playhead overlay outline
                if (isPlayhead) {
                    d.rect({ padX - 1, padY - 1 }, { padW + 2, padH + 2 }, 4, drawOpt(makeColor(255, 255, 255, 255)));
                }
            }
        }
    }

    void renderSoundEdit(Draw& d, ZicApp& app)
    {
        // Track selection tabs (y: 22..36)
        for (int t = 0; t < 4; ++t) {
            int tx = 4 + t * 78;
            bool isSel = (app.brain.selectedTrack == t);
            Color tabBg = isSel ? makeColor(0, 180, 220, 255) : makeColor(30, 36, 45, 255);
            d.filledRect({ tx, 22 }, { 74, 15 }, 3, drawOpt(tabBg));
            d.textCentered({ tx + 37, 24 }, app.brain.tracks[t].name, 10, textOpt(makeColor(255, 255, 255, 255)));
        }

        IEngine* eng = app.getSelectedEngine();
        if (!eng) return;

        int totalParams = (int)eng->getParamCount();
        int pageOffset = (app.activeParamIdx / 4) * 4;

        // Page info header (y: 38)
        char pageBuf[64];
        snprintf(pageBuf, sizeof(pageBuf), "PARAMS %d-%d OF %d", pageOffset + 1, std::min(pageOffset + 4, totalParams), totalParams);
        d.text({ 10, 39 }, pageBuf, 10, textOpt(makeColor(255, 200, 0, 255)));

        // Page nav [<] [>] buttons
        d.filledRect({ 250, 38 }, { 28, 15 }, 2, drawOpt(makeColor(50, 60, 75, 255)));
        d.textCentered({ 264, 40 }, "<", 10, textOpt(makeColor(255, 255, 255, 255)));

        d.filledRect({ 282, 38 }, { 28, 15 }, 2, drawOpt(makeColor(50, 60, 75, 255)));
        d.textCentered({ 296, 40 }, ">", 10, textOpt(makeColor(255, 255, 255, 255)));

        Param* params = eng->getParams();

        for (int i = 0; i < 4; ++i) {
            int pIdx = pageOffset + i;
            int paramY = 56 + i * 27;

            if (pIdx < totalParams) {
                bool isSel = (app.activeParamIdx == pIdx);

                Color bgCol = isSel ? makeColor(40, 50, 65, 255) : makeColor(20, 24, 30, 255);
                d.filledRect({ 10, paramY }, { 230, 25 }, 3, drawOpt(bgCol));

                if (isSel) {
                    d.rect({ 10, paramY }, { 230, 25 }, 3, drawOpt(makeColor(0, 220, 255, 255)));
                }

                Param& p = params[pIdx];
                // Param Name
                d.text({ 16, paramY + 6 }, p.label, 10, textOpt(makeColor(220, 230, 240, 255)));

                // Fill Bar
                float pct = (p.value - p.min) / (p.max - p.min);
                pct = std::clamp(pct, 0.0f, 1.0f);
                int barW = (int)(pct * 75.0f);
                d.filledRect({ 95, paramY + 6 }, { barW, 13 }, 2, drawOpt(makeColor(0, 180, 220, 255)));

                // Value Text
                char valBuf[32];
                snprintf(valBuf, sizeof(valBuf), "%.1f %s", p.value, p.unit ? p.unit : "");
                d.textRight({ 235, paramY + 6 }, valBuf, 9, textOpt(makeColor(255, 255, 255, 255)));

                // Adjust Buttons [-] [+]
                d.filledRect({ 245, paramY }, { 32, 25 }, 3, drawOpt(makeColor(50, 60, 75, 255)));
                d.textCentered({ 261, paramY + 6 }, "-", 11, textOpt(makeColor(255, 255, 255, 255)));

                d.filledRect({ 280, paramY }, { 32, 25 }, 3, drawOpt(makeColor(50, 60, 75, 255)));
                d.textCentered({ 296, paramY + 6 }, "+", 11, textOpt(makeColor(255, 255, 255, 255)));
            }
        }
    }

    void renderGlobal(Draw& d, ZicApp& app)
    {
        d.text({ 12, 23 }, "GLOBAL SETTINGS & PATTERNS", 11, textOpt(makeColor(255, 200, 0, 255)));

        // Play/Stop Toggle (y: 38..74, x: 10..155)
        Color playCol = app.brain.isPlaying ? makeColor(40, 160, 80, 255) : makeColor(160, 50, 50, 255);
        d.filledRect({ 10, 38 }, { 145, 36 }, 4, drawOpt(playCol));
        d.textCentered({ 82, 48 }, app.brain.isPlaying ? "PAUSE" : "PLAY", 14, textOpt(makeColor(255, 255, 255, 255)));

        // Generate Pattern (y: 38..74, x: 165..310)
        d.filledRect({ 165, 38 }, { 145, 36 }, 4, drawOpt(makeColor(0, 120, 180, 255)));
        d.textCentered({ 237, 48 }, "GEN PATTERN", 12, textOpt(makeColor(255, 255, 255, 255)));

        // BPM -5 / +5 (y: 84..120)
        d.filledRect({ 10, 84 }, { 145, 36 }, 4, drawOpt(makeColor(45, 55, 70, 255)));
        d.textCentered({ 82, 94 }, "BPM - 5", 12, textOpt(makeColor(255, 255, 255, 255)));

        d.filledRect({ 165, 84 }, { 145, 36 }, 4, drawOpt(makeColor(45, 55, 70, 255)));
        d.textCentered({ 237, 94 }, "BPM + 5", 12, textOpt(makeColor(255, 255, 255, 255)));

        // Touch gesture hint at bottom
        d.textCentered({ 160, 145 }, "<- Swipe left / right to change views ->", 10, textOpt(makeColor(130, 140, 150, 255)));
    }
};
