#pragma once

#include "draw/utils/Icon.h"
#include "draw/utils/inRect.h"
#include "zicXYv2/draw.h"
#include "zicXYv2/studio.h"
#include "zicXYv2/uiTopBar.h"

#include <chrono>
namespace UiSeq {

bool needsRedraw = true;

int ROW_H = 20;

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

bool draw(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY)
{
    const uint64_t initialDelayMs = 400;
    const uint64_t repeatIntervalMs = 80;
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

    if (leftHeld) {
        if (leftNextMoveMs == 0) leftNextMoveMs = nowMs + initialDelayMs;
        else if (nowMs >= leftNextMoveMs) {
            if (studio.selStep > 0) studio.selStep--;
            leftNextMoveMs = nowMs + repeatIntervalMs;
            needsRedraw = true;
        }
    } else {
        leftNextMoveMs = 0;
    }

    if (rightHeld) {
        if (rightNextMoveMs == 0) rightNextMoveMs = nowMs + initialDelayMs;
        else if (nowMs >= rightNextMoveMs) {
            if (studio.selStep < SEQ_STEPS - 1) studio.selStep++;
            rightNextMoveMs = nowMs + repeatIntervalMs;
            needsRedraw = true;
        }
    } else {
        rightNextMoveMs = 0;
    }

    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;

    top = currentY + 2;
    int tmpGridW = winW - (MARGIN * 2);
    stepW = std::max(2, tmpGridW / SEQ_STEPS);
    gridW = stepW * SEQ_STEPS;
    gridH = ROW_H * MAX_TRACKS;
    leftColW = winW - gridW - (MARGIN * 2);
    left = MARGIN + leftColW;

    // Background for the sequencer area
    d.filledRect({ left, top }, { gridW, gridH }, { .color = d.styles.colors.quaternary });

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

            const Step& st = trk.sequence[s];
            if (st.active) {
                Color c = trk.themeColor;
                // scale brightness by velocity
                float v = std::clamp(st.velocity, 0.0f, 1.0f);
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
            if (st.active) {
                // map note (24..96) to vertical position within bottom lane
                int noteMin = 24;
                int noteMax = 96;
                float clamped = std::clamp(st.note, noteMin, noteMax);
                float nm = 1.f - (clamped - (float)noteMin) / (float)(noteMax - noteMin);
                int laneH = botH;
                int ny = botR.position.y + botR.size.h - 1 - (int)(nm * (float)laneH);

                // compute line length in pixels (len in steps -> pixels). Clamp to grid end
                int maxRight = left + gridW;
                int lenPx = std::max(1, (int)std::round(st.len * (float)stepW));
                int x2 = std::min(maxRight - 1, x + lenPx - 1);

                d.line({ x, ny }, { x2, ny }, { .color = trk.themeColor });
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

    Param params[4] = {
        { .key = "note", .label = "Note", .value = haveSel ? (float)selStepRef.note : 60.0f, .min = 0.0f, .max = 127.0f, .step = 1.0f, .precision = 0 },
        { .key = "velocity", .label = "Velocity", .value = haveSel ? selStepRef.velocity : 1.0f, .min = 0.0f, .max = 1.0f, .step = 0.01f, .precision = 2 },
        { .key = "len", .label = "Len", .value = haveSel ? selStepRef.len : 1.0f, .min = 0.25f, .max = 64.25f, .step = 0.25f, .precision = 2 },
        { .key = "prob", .label = "Prob", .value = haveSel ? selStepRef.condition : 1.0f, .min = 0.0f, .max = 1.0f, .step = 0.01f, .precision = 2 },
    };
    for (auto& p : params)
        p.finalize();

    const int paramsPerRow = 4;
    const int colW = (winW - MARGIN * 2) / paramsPerRow;
    int currentY2 = paramsTopY;
    Color themeColor = { 0, 180, 255 };
    uint8_t encSel = 0;
    UiDraw::params(d, params, 4, winW, winH, colW, paramsTopY, paramsPerRow, currentY2, themeColor, encSel, 1);

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
        Step& st = studio.tracks[row]->sequence[s];
        st.active = !st.active;
    }

    needsRedraw = true;
    needFullRedraw = true;
}

void keyPressed(int key, bool& needFullRedraw)
{
    if (studio.currentCombinationKey == KeyView) return;
    
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
        std::lock_guard<std::mutex> lock(studio.audioMutex);
        Step& st = studio.tracks[studio.selTrack]->sequence[studio.selStep];
        st.active = !st.active;
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

// Encoders: 0=note,1=len,2=velocity,3=condition
void encoderTurned(int encoderIdx, int delta)
{
    if (studio.selTrack < 0 || studio.selStep < 0) return;
    if (studio.tracks[studio.selTrack] == nullptr) return;

    std::lock_guard<std::mutex> lock(studio.audioMutex);
    Step& st = studio.tracks[studio.selTrack]->sequence[studio.selStep];

    switch (encoderIdx) {
    case 0: // note
        st.note = std::clamp(st.note + delta, 0, 127);
        break;
    case 1: // len
        st.len = std::max(0.25f, st.len + delta * 0.25f);
        break;
    case 2: // velocity
        st.velocity = std::clamp(st.velocity + delta * 0.05f, 0.0f, 1.0f);
        break;
    case 3: // condition/prob
        st.condition = std::clamp(st.condition + delta * 0.05f, 0.0f, 1.0f);
        break;
    }

    needsRedraw = true;
}

// Handle mouse wheel for the parameter row under the sequencer
bool mouseWheelScrolled(Point position, int delta, const int winW, uint32_t now, bool shifted)
{
    if (studio.currentView != ViewSeq) return false;
    if (studio.selTrack < 0 || studio.selStep < 0) return false;
    if (studio.tracks[studio.selTrack] == nullptr) return false;

    const int paramsPerRow = 4;
    const int SB_WIDTH = 4;
    const int SB_GAP = 3;

    int usableWidth = winW - (MARGIN * 2) - (SB_WIDTH + SB_GAP);
    int adjustedColW = usableWidth / paramsPerRow;

    int visualRow = (position.y - paramsTopY) / UiDraw::ROW_H; // should be 0 for our single row
    int col = (position.x - MARGIN) / adjustedColW;

    const int maxVisibleRows = 1;

    if (visualRow >= 0 && visualRow < maxVisibleRows && col >= 0 && col < paramsPerRow) {
        int paramIdx = col; // 0=note,1=velocity,2=len,3=prob

        std::lock_guard<std::mutex> lock(studio.audioMutex);
        Step& st = studio.tracks[studio.selTrack]->sequence[studio.selStep];

        int scaled = encGetScaledDirection(delta, now, stepLastShiftTicks[paramIdx]);
        stepLastShiftTicks[paramIdx] = now;

        switch (paramIdx) {
        case 0: // note
            st.note = std::clamp(st.note + scaled, 0, 127);
            break;
        case 1: // velocity
            st.velocity = std::clamp(st.velocity + scaled * 0.05f, 0.0f, 1.0f);
            break;
        case 2: // len
            st.len = std::clamp(st.len + scaled * 0.25f, 0.25f, 64.25f);
            break;
        case 3: // prob / condition
            st.condition = std::clamp(st.condition + scaled * 0.05f, 0.0f, 1.0f);
            break;
        }

        needsRedraw = true;
        return true;
    }

    return false;
}

}
