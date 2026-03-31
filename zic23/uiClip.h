#pragma once

#include "zic23/studio.h"

sf::IntRect clipRects[MAX_TRACKS][32];
sf::IntRect saveBtnRects[MAX_TRACKS];
sf::IntRect loadBtnRects[MAX_TRACKS];

static constexpr int CLIP_H = 12;

void saveClip(int trackIdx, int clipIdx)
{
    Track& trk = *studio.tracks[trackIdx];
    Clip& c = trk.clips[clipIdx];
    c.paramValues.clear();
    Param* params = trk.engine->getParams();
    for (size_t i = 0; i < trk.engine->getParamCount(); i++) {
        c.paramValues.push_back(params[i].value);
    }
    c.saved = true;
    c.sequence = trk.sequence;
    trk.activeClipIdx = clipIdx;
}

void loadClip(int trackIdx, int clipIdx)
{
    Track& trk = *studio.tracks[trackIdx];
    Clip& c = trk.clips[clipIdx];

    Param* params = trk.engine->getParams();
    for (size_t i = 0; i < c.paramValues.size(); i++) {
        params[i].value = c.paramValues[i];
    }
    trk.sequence = c.sequence;
    trk.activeClipIdx = clipIdx;
}

void drawClipSelectorUI(Draw& d, sf::Vector2u size, int currentY, int clipStartX, int trackIdx, Track& trk)
{
    int btnX = size.x - MARGIN * 2 - 15;
    saveBtnRects[trackIdx] = { btnX, currentY + 4, 15, 12 };
    d.filledRect({ saveBtnRects[trackIdx].left, saveBtnRects[trackIdx].top }, { 15, 12 }, { .color = { 60, 60, 70 } });
    d.text({ saveBtnRects[trackIdx].left + 4, saveBtnRects[trackIdx].top + 1 }, "S", 8, { .color = { 200, 200, 200 }, .font = &PoppinsLight_8 });

    btnX -= 4 + 15;
    loadBtnRects[trackIdx] = { btnX, currentY + 4, 15, 12 };
    d.filledRect({ loadBtnRects[trackIdx].left, loadBtnRects[trackIdx].top }, { 15, 12 }, { .color = { 60, 60, 70 } });
    d.text({ loadBtnRects[trackIdx].left + 4, loadBtnRects[trackIdx].top + 1 }, "L", 8, { .color = { 200, 200, 200 }, .font = &PoppinsLight_8 });

    int CLIP_W = (btnX - clipStartX - MARGIN) / 32 - 2;
    for (int i = 0; i < 32; i++) {
        clipRects[trackIdx][i] = { clipStartX + i * (CLIP_W + 2), currentY + 4, CLIP_W, CLIP_H };
        Color c = trk.themeColor;
        if (trk.activeClipIdx != i) {
            if (trk.clips[i].saved) {
                c.a = 100;
            } else {
                c = Color { 40, 40, 45 };
            }
        }
        d.filledRect({ clipRects[trackIdx][i].left, clipRects[trackIdx][i].top }, { CLIP_W, CLIP_H }, { .color = c });

        if (trk.selectedClipIdx == i) d.filledRect({ clipRects[trackIdx][i].left, clipRects[trackIdx][i].top + CLIP_H - 3 }, { CLIP_W, 3 }, { .color = { 255, 255, 255 } });
    }
}