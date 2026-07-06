#pragma once

#include "draw/utils/Icon.h"
#include "draw/utils/inRect.h"
#include "helpers/midiNote.h"
#include "zicXYv2/draw.h"
#include "zicXYv2/studio.h"
#include "zicXYv2/uiTopBar.h"
#include "zicXYv2/utils.h"

#include <chrono>
namespace UiSeq {

bool needsRedraw = true;

static constexpr int ROW_H = 20;

int top = 0;
int stepW = 0;
int gridW = 0;
int gridH = 0;
int leftColW = 0;
int left = 0;
// hold state for left/right (UI-agnostic)
static bool leftHeld = false;
static bool rightHeld = false;
static uint64_t leftNextMoveMs = 0;
static uint64_t rightNextMoveMs = 0;
// parameter panel Y (for mouse interactions)
int paramsTopY = 0;

// last tick times for mouse-wheel scaling for each of the 4 params
static uint32_t stepLastShiftTicks[4] = { 0, 0, 0, 0 };

// simple incremental playhead state: remember last X position to avoid redraws
static int lastPlayheadPosition = -1;
static Color playheadSaved[ROW_H * MAX_TRACKS];
static bool playheadSavedActive[MAX_TRACKS] = { false };

// Incremental updater: on step change, restore previous vertical playhead and
// draw a new vertical line for each visible track.
static bool drawPlayheadIncremental(Draw& d, bool skipRestore = false)
{
    int currentStep = studio.isPlaying ? (studio.currentStep % SEQ_STEPS) : -1;
    int currentPlayheadPosition = (currentStep >= 0) ? left + currentStep * stepW : -1;
    if (!skipRestore && currentPlayheadPosition == lastPlayheadPosition) return false;

    bool rendered = false;

    // Restore previous vertical playhead for each track
    if (!skipRestore && lastPlayheadPosition >= 0) {
        for (int t = 0; t < MAX_TRACKS; t++) {
            if (studio.tracks[t] == nullptr) break;
            if (!playheadSavedActive[t]) continue;
            int y = top + t * ROW_H;
            for (int yy = 0; yy < ROW_H; yy++) {
                d.pixel({ lastPlayheadPosition, y + yy }, playheadSaved[t * ROW_H + yy]);
            }
            playheadSavedActive[t] = false;
            rendered = true;
        }
    }

    // if stopped, we're done
    if (currentPlayheadPosition < 0) {
        lastPlayheadPosition = -1;
        return rendered;
    }

    // Draw vertical playhead and save underlying pixels
    for (int t = 0; t < MAX_TRACKS; t++) {
        if (studio.tracks[t] == nullptr) break;
        Track& trk = *studio.tracks[t];

        int y = top + t * ROW_H;
        int sx = currentPlayheadPosition;
        playheadSavedActive[t] = true;

        for (int yy = 0; yy < ROW_H; yy++) {
            int py = y + yy;
            playheadSaved[t * ROW_H + yy] = d.getPixel({ sx, py });
            d.pixel({ sx, py }, Color { 255, 255, 255 });
        }

        rendered = true;
    }

    lastPlayheadPosition = currentPlayheadPosition;
    return rendered;
}

bool draw(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY)
{
    const uint64_t initialDelayMs = 400;
    const uint64_t repeatIntervalMs = 80;
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

    // Keyboard left/right arrow handling repeat...
    if (leftHeld) {
        if (leftNextMoveMs == 0) leftNextMoveMs = nowMs + initialDelayMs;
        else if (nowMs >= leftNextMoveMs) {
            if (studio.selStep > 0) studio.selStep--;
            leftNextMoveMs = nowMs + repeatIntervalMs;
            needsRedraw = true;
        }
    }

    if (rightHeld) {
        if (rightNextMoveMs == 0) rightNextMoveMs = nowMs + initialDelayMs;
        else if (nowMs >= rightNextMoveMs) {
            if (studio.selStep < SEQ_STEPS - 1) studio.selStep++;
            rightNextMoveMs = nowMs + repeatIntervalMs;
            needsRedraw = true;
        }
    }

    // compute layout first so incremental updater knows positions
    top = currentY + 2;
    int tmpGridW = winW - (MARGIN * 2);
    stepW = std::max(2, tmpGridW / SEQ_STEPS);
    gridW = stepW * SEQ_STEPS;
    gridH = ROW_H * MAX_TRACKS;
    leftColW = winW - gridW - (MARGIN * 2);
    left = MARGIN + leftColW;

    // if only incremental update needed, do it
    if (!needsRedraw && !needFullRedraw) {
        return drawPlayheadIncremental(d);
    }
    needsRedraw = false;

    // Background for the sequencer area
    d.filledRect({ left - 1, top - 1 }, { gridW + 2, gridH + 1 }, { .color = d.styles.colors.quaternary });

    // Left column background (track names + mute)
    // Left column background (track names + mute)
    d.filledRect({ MARGIN, top }, { leftColW, gridH }, { .color = d.styles.colors.background });

    // Icon helper
    Icon icon(d);

    int rowH = ROW_H - 2;

    // Draw left column (track name + mute) and grid of steps
    for (int t = 0; t < MAX_TRACKS; t++) {
        if (studio.tracks[t] == nullptr) break;
        Track& trk = *studio.tracks[t];
        int y = top + t * ROW_H;

        // Draw track name in the left column
        d.filledRect({ MARGIN, y }, { leftColW, ROW_H }, { .color = { 30, 30, 30 } });
        d.text({ MARGIN + 6, y + 4 }, "Track " + std::to_string(t + 1), 8, { .color = trk.themeColor, .font = &PoppinsLight_8 });

        // Draw mute icon if muted
        if (trk.isMuted) {
            icon.mute({ MARGIN + leftColW - 14, y + 4 }, { 10, 10 }, { 155, 155, 155 }, true);
        }

        for (int s = 0; s < SEQ_STEPS; s++) {
            int x = left + s * stepW;
            // split the row into two halves: top half for selection/activation, bottom half for length/note display
            int halfH = rowH / 2;
            int topH = halfH;
            int botH = rowH - halfH;
            Rect topR = { { x, y }, { stepW - 1, topH } };
            Rect botR = { { x, y + topH }, { stepW - 1, botH } };

            const Step& step = trk.sequence[s];
            if (step.active) {
                Color c = trk.themeColor;
                // scale brightness by velocity
                float v = std::clamp(step.velocity, 0.0f, 1.0f);
                c.r = std::min(255, (int)(c.r * (0.4f + 0.6f * v)));
                c.g = std::min(255, (int)(c.g * (0.4f + 0.6f * v)));
                c.b = std::min(255, (int)(c.b * (0.4f + 0.6f * v)));
                d.filledRect(topR.position, topR.size, { .color = c });
            } else {
                d.rect(topR.position, topR.size, { .color = { 255, 255, 255, 8 } });
            }

            // highlight playhead (keep it at the bottom of the top half)
            if (studio.currentStep % SEQ_STEPS == s) {
                d.filledRect({ x, y + topH - 2 }, { stepW, 2 }, { .color = { 255, 255, 255, 30 } });
            }

            // selected step outline (around the top half)
            if (studio.selTrack == t && studio.selStep == s) {
                d.rect({ x - 1, y - 1 }, { stepW + 1, topH + 2 }, { .color = { 255, 255, 255 } });
            }

            // draw length/note line in the bottom half for active steps
            if (step.active) {
                // map note (24..96) to vertical position within bottom lane
                int noteMin = 24;
                int noteMax = 96;
                float clamped = std::clamp(step.note, noteMin, noteMax);
                float nm = 1.f - (clamped - (float)noteMin) / (float)(noteMax - noteMin);
                int laneH = botH;
                int ny = botR.position.y + botR.size.h - 1 - (int)(nm * (float)laneH);

                // compute line length in pixels (len in steps -> pixels).
                // If the line extends past the right edge of the grid, wrap it
                // back to the left side so long steps are shown across the boundary.
                int maxRight = left + gridW;
                int lenPx = std::max(1, (int)std::round(step.len * (float)stepW));

                int remaining = lenPx;
                int curX = x;
                // draw in segments: from current x to grid end, then wrap to left
                while (remaining > 0) {
                    int space = maxRight - curX; // pixels available until grid end
                    if (space <= 0) break;
                    int drawLen = std::min(remaining, space);
                    int xEnd = curX + drawLen - 1;
                    d.line({ curX, ny }, { xEnd, ny }, { .color = trk.themeColor });
                    remaining -= drawLen;
                    // after first segment, wrap to the grid left
                    curX = left;
                    // if we've wrapped and still have more than a full grid, avoid infinite loop
                    if (drawLen == 0) break;
                    // if remaining is larger than a full grid width, cap remaining to at most gridW
                    if (remaining > gridW) {
                        // draw a full-width line once and reduce remaining accordingly
                        d.line({ left, ny }, { maxRight - 1, ny }, { .color = trk.themeColor });
                        remaining -= gridW;
                    }
                }
            }
        }
    }

    // Parameter panel under the sequencer: Note, Velocity, Len, Probability
    paramsTopY = top + gridH + 6;
    Step selStepRef;
    bool haveSel = false;
    if (studio.selTrack >= 0 && studio.selTrack < MAX_TRACKS && studio.tracks[studio.selTrack] != nullptr && studio.selStep >= 0 && studio.selStep < SEQ_STEPS) {
        selStepRef = studio.tracks[studio.selTrack]->sequence[studio.selStep];
        haveSel = true;
    }

    uint8_t note = haveSel ? selStepRef.note : 60;
    Param params[4] = {
        { .key = "note", .label = "Note", .string = (char*)MIDI_NOTES_STR[note], .value = (float)note, .min = 0.0f, .max = 127.0f, .step = 1.0f, .precision = 0 },
        { .key = "velocity", .label = "Velocity", .unit = "%", .value = haveSel ? selStepRef.velocity * 100 : 100.0f },
        { .key = "len", .label = "Len", .value = haveSel ? selStepRef.len : 1.0f, .min = 0.25f, .max = 64.25f, .step = 0.25f, .precision = 2 },
        { .key = "prob", .label = "Prob", .unit = "%", .value = haveSel ? selStepRef.condition * 100 : 100.0f },
    };
    for (auto& p : params)
        p.finalize();

    Color themeColor = { 0, 180, 255 };
    uint8_t encSel = 0;
    UiDraw::params(d, params, 4, winW, winH, paramsTopY, 4, themeColor, encSel, 1);

    drawPlayheadIncremental(d, true);
    return true;
}

void mouseButtonPressed(Point position, const int winW, bool& needFullRedraw)
{
    if (position.x < left || position.x > left + gridW) return;
    int s = (position.x - left) / stepW;
    const int leftColW = 72; // space for track name + mute icon
    const int left = MARGIN + leftColW;
    int row = (position.y - top) / ROW_H;
    if (s < 0 || s >= SEQ_STEPS) return;
    if (row < 0 || row >= MAX_TRACKS) return;

    studio.selTrack = row;
    studio.selStep = s;

    {
        std::lock_guard<std::mutex> lock(studio.audioMutex);
        Step& step = studio.tracks[row]->sequence[s];
        step.active = !step.active;
    }

    needsRedraw = true;
    needFullRedraw = true;
}

void keyPressed(int key, bool& needFullRedraw)
{
    if (studio.currentCombinationKey == KeyView) return;
    if (studio.currentView != ViewSeq) return;

    if (key == KEY_1) { // Left
        if (studio.selStep > 0) studio.selStep--;
        // start hold
        leftHeld = true;
        // needFullRedraw = true;
        needsRedraw = true;
    } else if (key == KEY_3) { // Right
        if (studio.selStep < SEQ_STEPS - 1) studio.selStep++;
        // start hold
        rightHeld = true;
        // needFullRedraw = true;
        needsRedraw = true;
    } else if (key == KEY_F2) { // Up
        if (studio.selTrack > 0) studio.selTrack--;
        // needFullRedraw = true;
        needsRedraw = true;
    } else if (key == KEY_2) { // Down
        if (studio.selTrack < MAX_TRACKS - 1) studio.selTrack++;
        // needFullRedraw = true;
        needsRedraw = true;
    } else if (key == KEY_F3) { // Toggle
        Track& trk = *studio.tracks[studio.selTrack];
        Step& step = trk.sequence[studio.selStep];
        {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            step.active = !step.active;
        }
        if (step.active) {
            triggerPreview(trk, step.note, step.velocity);
        }
        needsRedraw = true;
    }
}

void keyReleased(int key, bool& needFullRedraw)
{
    if (key == KEY_1) {
        leftHeld = false;
    } else if (key == KEY_3) {
        rightHeld = false;
    }
}

void onEncoder(int encoderId, int8_t direction)
{
    if (studio.currentView != ViewSeq) return;
    if (direction == 0) return;
    if (studio.selTrack < 0 || studio.selStep < 0) return;
    if (studio.tracks[studio.selTrack] == nullptr) return;

    int paramIdx = std::clamp(encoderId - 1, 0, 3); // 0=note,1=velocity,2=len,3=prob
    Track& trk = *studio.tracks[studio.selTrack];
    Step& step = trk.sequence[studio.selStep];

    uint32_t now = (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch())
                       .count();
    int scaled = encGetScaledDirection(direction, now, stepLastShiftTicks[paramIdx]);
    stepLastShiftTicks[paramIdx] = now;

    switch (paramIdx) {
    case 0: {
        std::unique_lock<std::mutex> lock(studio.audioMutex, std::try_to_lock);
        if (!lock.owns_lock()) return;
        step.note = std::clamp(step.note + scaled, 0, 127);
    }
        triggerPreview(trk, step.note, step.velocity);
        break;
    case 1: {
        std::unique_lock<std::mutex> lock(studio.audioMutex, std::try_to_lock);
        if (!lock.owns_lock()) return;
        step.velocity = std::clamp(step.velocity + scaled * 0.05f, 0.0f, 1.0f);
    } break;
    case 2: {
        std::unique_lock<std::mutex> lock(studio.audioMutex, std::try_to_lock);
        if (!lock.owns_lock()) return;
        step.len = std::clamp(step.len + scaled * 0.25f, 0.25f, 64.25f);
    } break;
    case 3: {
        std::unique_lock<std::mutex> lock(studio.audioMutex, std::try_to_lock);
        if (!lock.owns_lock()) return;
        step.condition = std::clamp(step.condition + scaled * 0.05f, 0.0f, 1.0f);
    } break;
    }

    needsRedraw = true;
}

// Handle mouse wheel for the parameter row under the sequencer
bool mouseWheelScrolled(Point position, int delta, const int winW, uint32_t now, bool shifted)
{
    if (studio.currentView != ViewSeq) return false;

    const int paramsPerRow = 4;
    const int SB_WIDTH = 4;
    const int SB_GAP = 3;

    int usableWidth = winW - (MARGIN * 2) - (SB_WIDTH + SB_GAP);
    int adjustedColW = usableWidth / paramsPerRow;

    int visualRow = (position.y - paramsTopY) / UiDraw::ROW_H; // should be 0 for our single row
    int col = (position.x - MARGIN) / adjustedColW;

    const int maxVisibleRows = 1;

    if (visualRow >= 0 && visualRow < maxVisibleRows && col >= 0 && col < paramsPerRow) {
        onEncoder(col + 1, delta);
        return true;
    }

    return false;
}

}
