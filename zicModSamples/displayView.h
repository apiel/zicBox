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
        // Touch move handling
    }

    void handleTouchUp(int x, int y, ZicApp& app)
    {
        if (!isTouching) return;
        isTouching = false;

        int deltaX = x - touchDownX;
        int deltaY = y - touchDownY;

        // Detect Horizontal Swipe (View Switch)
        if (std::abs(deltaX) > 40 && std::abs(deltaY) < 30) {
            if (deltaX < 0) {
                app.nextView();
            } else {
                app.prevView();
            }
            return;
        }

        // Tap Handlers
        if (std::abs(deltaX) < 15 && std::abs(deltaY) < 15) {
            // Top Bar View Dots Tap (x: 250..315, y: 0..20)
            if (y <= 20) {
                if (x >= 250 && x <= 315) {
                    int v = (x - 250) / 16;
                    app.setView(v);
                    return;
                }
            }

            if (app.currentView == VIEW_OVERVIEW) {
                // Overview: 8 track rows
                for (int t = 0; t < SequenceBrain::NUM_TRACKS; ++t) {
                    int rowY = 22 + t * 18;
                    if (y >= rowY && y <= rowY + 17) {
                        if (x >= 65 && x <= 115) {
                            app.brain.toggleMute(t);
                        } else {
                            app.brain.selectedTrack = t;
                            app.sampleTracks[t].trigger();
                        }
                        return;
                    }
                }
            } else if (app.currentView == VIEW_STEP_EDIT) {
                // Track tabs (y: 22..36)
                if (y >= 22 && y <= 36) {
                    int t = (x - 4) / 39;
                    if (t >= 0 && t < SequenceBrain::NUM_TRACKS) {
                        app.brain.selectedTrack = t;
                        return;
                    }
                }
                // 4x4 Step Grid (y: 40..165)
                int padW = 70;
                int padH = 27;
                int startX = 12;
                int startY = 40;
                for (int r = 0; r < 4; ++r) {
                    for (int c = 0; c < 4; ++c) {
                        int stepIdx = r * 4 + c;
                        int px = startX + c * (padW + 6);
                        int py = startY + r * (padH + 4);
                        if (x >= px && x <= px + padW && y >= py && y <= py + padH) {
                            app.brain.toggleStep(app.brain.selectedTrack, stepIdx);
                            return;
                        }
                    }
                }
            } else if (app.currentView == VIEW_SOUND_EDIT) {
                // Track tabs (y: 22..36)
                if (y >= 22 && y <= 36) {
                    int t = (x - 4) / 39;
                    if (t >= 0 && t < SequenceBrain::NUM_TRACKS) {
                        app.brain.selectedTrack = t;
                        return;
                    }
                }
                // Sample Select [-] [+] (y: 45..75)
                if (y >= 45 && y <= 75) {
                    if (x >= 240 && x <= 275) {
                        uint8_t curr = app.sampleTracks[app.brain.selectedTrack].sampleIdx;
                        app.sampleTracks[app.brain.selectedTrack].setSample((curr + 7) % 8);
                        return;
                    }
                    if (x >= 280 && x <= 315) {
                        uint8_t curr = app.sampleTracks[app.brain.selectedTrack].sampleIdx;
                        app.sampleTracks[app.brain.selectedTrack].setSample((curr + 1) % 8);
                        return;
                    }
                }
                // Pitch [-] [+] (y: 80..110)
                if (y >= 80 && y <= 110) {
                    if (x >= 240 && x <= 275) {
                        app.sampleTracks[app.brain.selectedTrack].pitch = std::clamp(app.sampleTracks[app.brain.selectedTrack].pitch - 1.0f, -12.0f, 12.0f);
                        app.sampleTracks[app.brain.selectedTrack].updateSpeed();
                        return;
                    }
                    if (x >= 280 && x <= 315) {
                        app.sampleTracks[app.brain.selectedTrack].pitch = std::clamp(app.sampleTracks[app.brain.selectedTrack].pitch + 1.0f, -12.0f, 12.0f);
                        app.sampleTracks[app.brain.selectedTrack].updateSpeed();
                        return;
                    }
                }
                // Volume [-] [+] (y: 115..145)
                if (y >= 115 && y <= 145) {
                    if (x >= 240 && x <= 275) {
                        app.sampleTracks[app.brain.selectedTrack].volume = std::clamp(app.sampleTracks[app.brain.selectedTrack].volume - 0.1f, 0.0f, 1.0f);
                        return;
                    }
                    if (x >= 280 && x <= 315) {
                        app.sampleTracks[app.brain.selectedTrack].volume = std::clamp(app.sampleTracks[app.brain.selectedTrack].volume + 0.1f, 0.0f, 1.0f);
                        return;
                    }
                }
            } else if (app.currentView == VIEW_GLOBAL) {
                // Play / Stop (y: 38..74, x: 10..155)
                if (y >= 38 && y <= 74 && x >= 10 && x <= 155) {
                    app.brain.isPlaying = !app.brain.isPlaying;
                    return;
                }
                // Gen Pattern (y: 38..74, x: 165..310)
                if (y >= 38 && y <= 74 && x >= 165 && x <= 310) {
                    app.brain.generatePattern();
                    return;
                }
                // BPM -5 (y: 84..120, x: 10..155)
                if (y >= 84 && y <= 120 && x >= 10 && x <= 155) {
                    app.brain.setBpm(app.brain.bpm - 5.0f);
                    return;
                }
                // BPM +5 (y: 84..120, x: 165..310)
                if (y >= 84 && y <= 120 && x >= 165 && x <= 310) {
                    app.brain.setBpm(app.brain.bpm + 5.0f);
                    return;
                }
            }
        }
    }

    void render(Draw& d, ZicApp& app)
    {
        d.clear();

        // Top Navigation Bar
        d.filledRect({ 0, 0 }, { SCREEN_W, 20 }, drawOpt(makeColor(24, 28, 36, 255)));

        char topBuf[64];
        const char* viewNames[NUM_VIEWS] = { "OVERVIEW (8-TRK)", "STEP SEQUENCER", "SAMPLE EDITOR", "GLOBAL / BPM" };
        snprintf(topBuf, sizeof(topBuf), "%s", viewNames[(int)app.currentView]);
        d.text({ 8, 4 }, topBuf, 12, textOpt(makeColor(0, 220, 255, 255)));

        snprintf(topBuf, sizeof(topBuf), "%d BPM  %s", (int)app.brain.bpm, app.brain.isPlaying ? "RUN" : "STOP");
        d.text({ 140, 4 }, topBuf, 12, textOpt(app.brain.isPlaying ? makeColor(0, 255, 128, 255) : makeColor(255, 100, 100, 255)));

        // 4 View Indicator Dots
        for (int i = 0; i < NUM_VIEWS; ++i) {
            Color dotCol = ((int)app.currentView == i) ? makeColor(0, 220, 255, 255) : makeColor(80, 90, 100, 255);
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
    void renderOverview(Draw& d, ZicApp& app)
    {
        for (int t = 0; t < SequenceBrain::NUM_TRACKS; ++t) {
            int rowY = 22 + t * 18;
            bool isSel = (app.brain.selectedTrack == t);
            bool isMuted = app.brain.tracks[t].muted;

            Color bgCol = isSel ? makeColor(40, 50, 65, 255) : makeColor(18, 22, 28, 255);
            d.filledRect({ 4, rowY }, { 312, 17 }, 2, drawOpt(bgCol));

            if (isSel) {
                d.rect({ 4, rowY }, { 312, 17 }, 2, drawOpt(makeColor(0, 200, 255, 255)));
            }

            // Track Name
            Color nameCol = isSel ? makeColor(255, 255, 255, 255) : makeColor(180, 190, 200, 255);
            d.text({ 8, rowY + 3 }, app.brain.tracks[t].name, 9, textOpt(nameCol));

            // Mute Button
            Color muteBg = isMuted ? makeColor(180, 40, 40, 255) : makeColor(30, 140, 70, 255);
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
                    stepCol = isMuted ? makeColor(120, 120, 60, 255) : makeColor(0, 220, 160, 255);
                } else {
                    stepCol = makeColor(40, 45, 55, 255);
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
            Color tabBg = isSel ? makeColor(0, 180, 220, 255) : makeColor(30, 36, 45, 255);
            d.filledRect({ tx, 22 }, { 36, 15 }, 2, drawOpt(tabBg));
            d.textCentered({ tx + 18, 24 }, app.brain.tracks[t].name, 8, textOpt(makeColor(255, 255, 255, 255)));
        }

        int trk = app.brain.selectedTrack;
        DrumTrack& track = app.brain.tracks[trk];

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

                char numBuf[8];
                snprintf(numBuf, sizeof(numBuf), "%d", stepIdx + 1);
                Color textCol = active ? makeColor(255, 255, 255, 255) : makeColor(120, 130, 140, 255);
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
            Color tabBg = isSel ? makeColor(0, 180, 220, 255) : makeColor(30, 36, 45, 255);
            d.filledRect({ tx, 22 }, { 36, 15 }, 2, drawOpt(tabBg));
            d.textCentered({ tx + 18, 24 }, app.brain.tracks[t].name, 8, textOpt(makeColor(255, 255, 255, 255)));
        }

        int trk = app.brain.selectedTrack;
        SampleTrack& sTrack = app.sampleTracks[trk];

        // Control 1: Sample Select
        d.filledRect({ 10, 45 }, { 220, 30 }, 4, drawOpt(makeColor(24, 30, 40, 255)));
        d.text({ 16, 53 }, "SAMPLE", 11, textOpt(makeColor(200, 210, 220, 255)));
        d.textRight({ 220, 53 }, g_presetSamples[sTrack.sampleIdx].name, 10, textOpt(makeColor(0, 220, 255, 255)));
        d.filledRect({ 240, 45 }, { 35, 30 }, 4, drawOpt(makeColor(50, 60, 75, 255)));
        d.textCentered({ 257, 53 }, "-", 12, textOpt(makeColor(255, 255, 255, 255)));
        d.filledRect({ 280, 45 }, { 35, 30 }, 4, drawOpt(makeColor(50, 60, 75, 255)));
        d.textCentered({ 297, 53 }, "+", 12, textOpt(makeColor(255, 255, 255, 255)));

        // Control 2: Pitch
        d.filledRect({ 10, 80 }, { 220, 30 }, 4, drawOpt(makeColor(24, 30, 40, 255)));
        d.text({ 16, 88 }, "PITCH", 11, textOpt(makeColor(200, 210, 220, 255)));
        char pBuf[32];
        snprintf(pBuf, sizeof(pBuf), "%+.0f ST", sTrack.pitch);
        d.textRight({ 220, 88 }, pBuf, 10, textOpt(makeColor(255, 200, 0, 255)));
        d.filledRect({ 240, 80 }, { 35, 30 }, 4, drawOpt(makeColor(50, 60, 75, 255)));
        d.textCentered({ 257, 88 }, "-", 12, textOpt(makeColor(255, 255, 255, 255)));
        d.filledRect({ 280, 80 }, { 35, 30 }, 4, drawOpt(makeColor(50, 60, 75, 255)));
        d.textCentered({ 297, 88 }, "+", 12, textOpt(makeColor(255, 255, 255, 255)));

        // Control 3: Volume
        d.filledRect({ 10, 115 }, { 220, 30 }, 4, drawOpt(makeColor(24, 30, 40, 255)));
        d.text({ 16, 123 }, "VOLUME", 11, textOpt(makeColor(200, 210, 220, 255)));
        char vBuf[32];
        snprintf(vBuf, sizeof(vBuf), "%d%%", (int)(sTrack.volume * 100.0f));
        d.textRight({ 220, 123 }, vBuf, 10, textOpt(makeColor(0, 255, 160, 255)));
        d.filledRect({ 240, 115 }, { 35, 30 }, 4, drawOpt(makeColor(50, 60, 75, 255)));
        d.textCentered({ 257, 123 }, "-", 12, textOpt(makeColor(255, 255, 255, 255)));
        d.filledRect({ 280, 115 }, { 35, 30 }, 4, drawOpt(makeColor(50, 60, 75, 255)));
        d.textCentered({ 297, 123 }, "+", 12, textOpt(makeColor(255, 255, 255, 255)));
    }

    void renderGlobal(Draw& d, ZicApp& app)
    {
        d.text({ 12, 23 }, "GLOBAL SETTINGS & PATTERNS", 11, textOpt(makeColor(255, 200, 0, 255)));

        Color playCol = app.brain.isPlaying ? makeColor(40, 160, 80, 255) : makeColor(160, 50, 50, 255);
        d.filledRect({ 10, 38 }, { 145, 36 }, 4, drawOpt(playCol));
        d.textCentered({ 82, 48 }, app.brain.isPlaying ? "PAUSE" : "PLAY", 14, textOpt(makeColor(255, 255, 255, 255)));

        d.filledRect({ 165, 38 }, { 145, 36 }, 4, drawOpt(makeColor(0, 120, 180, 255)));
        d.textCentered({ 237, 48 }, "GEN PATTERN", 12, textOpt(makeColor(255, 255, 255, 255)));

        d.filledRect({ 10, 84 }, { 145, 36 }, 4, drawOpt(makeColor(45, 55, 70, 255)));
        d.textCentered({ 82, 94 }, "BPM - 5", 12, textOpt(makeColor(255, 255, 255, 255)));

        d.filledRect({ 165, 84 }, { 145, 36 }, 4, drawOpt(makeColor(45, 55, 70, 255)));
        d.textCentered({ 237, 94 }, "BPM + 5", 12, textOpt(makeColor(255, 255, 255, 255)));

        d.textCentered({ 160, 145 }, "<- Swipe left / right to change views ->", 10, textOpt(makeColor(130, 140, 150, 255)));
    }
};
