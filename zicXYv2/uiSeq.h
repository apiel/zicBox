#pragma once

#include <SFML/Window/Keyboard.hpp>

#include "zicXYv2/draw.h"
#include "zicXYv2/studio.h"
#include "zicXYv2/uiTopBar.h"
#include "draw/utils/inRect.h"

namespace UiSeq {

bool needsRedraw = true;

int ROW_H = 18;

bool draw(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY)
{
    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;

    const int left = MARGIN;
    const int top = currentY + 2;
    const int gridW = winW - (MARGIN * 2);
    const int gridH = ROW_H * MAX_TRACKS;

    d.filledRect({ left, top }, { gridW, gridH }, { .color = d.styles.colors.quaternary });

    int stepW = std::max(2, gridW / SEQ_STEPS);
    int rowH = ROW_H - 2;

    // Draw grid of steps
    for (int t = 0; t < MAX_TRACKS; t++) {
        if (studio.tracks[t] == nullptr) break;
        Track& trk = *studio.tracks[t];
        int y = top + t * ROW_H;

        for (int s = 0; s < SEQ_STEPS; s++) {
            int x = left + s * stepW;
            Rect r = { { x, y }, { stepW - 1, rowH } };

            const Step& st = trk.sequence[s];
            if (st.active) {
                Color c = trk.themeColor;
                // scale brightness by velocity
                float v = std::clamp(st.velocity, 0.0f, 1.0f);
                c.r = std::min(255, (int)(c.r * (0.4f + 0.6f * v)));
                c.g = std::min(255, (int)(c.g * (0.4f + 0.6f * v)));
                c.b = std::min(255, (int)(c.b * (0.4f + 0.6f * v)));
                d.filledRect(r.position, r.size, { .color = c });
            } else {
                d.rect(r.position, r.size, { .color = { 255, 255, 255, 8 } });
            }

            // highlight playhead
            if (studio.currentStep % SEQ_STEPS == s) {
                d.filledRect({ x, y + rowH - 2 }, { stepW, 2 }, { .color = { 255, 255, 255, 30 } });
            }

            // selected step outline
            if (studio.selTrack == t && studio.selStep == s) {
                d.rect({ x - 1, y - 1 }, { stepW + 1, rowH + 2 }, { .color = { 255, 255, 255 } });
            }
        }
    }

    return true;
}

void mouseButtonPressed(Point position, bool& needFullRedraw)
{
    // Toggle step if clicked
    int left = MARGIN;
    int top = TopBar::height; // use TopBar's height
    int gridW = SCREEN_W - (MARGIN * 2);
    int stepW = std::max(2, gridW / SEQ_STEPS);

    if (position.x < left || position.x > left + gridW) return;
    int s = (position.x - left) / stepW;
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
    using K = sf::Keyboard;

    if (key == K::Left) {
        if (studio.selStep > 0) studio.selStep--;
        needFullRedraw = true;
        needsRedraw = true;
    } else if (key == K::Right) {
        if (studio.selStep < SEQ_STEPS - 1) studio.selStep++;
        needFullRedraw = true;
        needsRedraw = true;
    } else if (key == K::Up) {
        if (studio.selTrack > 0) studio.selTrack--;
        needFullRedraw = true;
        needsRedraw = true;
    } else if (key == K::Down) {
        if (studio.selTrack < MAX_TRACKS - 1) studio.selTrack++;
        needFullRedraw = true;
        needsRedraw = true;
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

void buttonPressed(int btnIdx)
{
    if (studio.selTrack < 0 || studio.selStep < 0) return;
    if (studio.tracks[studio.selTrack] == nullptr) return;

    if (btnIdx == 0) { // toggle active
        std::lock_guard<std::mutex> lock(studio.audioMutex);
        Step& st = studio.tracks[studio.selTrack]->sequence[studio.selStep];
        st.active = !st.active;
        needsRedraw = true;
    }
}

}
