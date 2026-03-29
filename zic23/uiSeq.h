#pragma once

#include "helpers/format.h"
#include "helpers/midiNote.h"
#include "zic23/studio.h"

int drawMixerUI(Draw& d, sf::Vector2u size, int currentY)
{
    const int winW = (int)size.x;
    const int muteW = 25, volW = 70, genW = 25, sp = 5;
    const int mixW = muteW + sp + volW + sp + sp + genW + 10;
    const int stepW = (winW - (MARGIN * 2 + mixW)) / 64;

    for (int i = 0; i < MAX_TRACKS; i++) {
        Track& trk = *studio.tracks[i];
        trk.muteRect = { MARGIN, currentY, muteW, STEP_H };
        d.filledRect({ trk.muteRect.left, trk.muteRect.top }, { muteW, STEP_H },
            { .color = trk.isMuted ? Color { 200, 50, 50 } : Color { 40, 40, 45 } });
        d.text({ trk.muteRect.left + 8, trk.muteRect.top + 1 }, "M", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        trk.volRect = { trk.muteRect.left + muteW + sp, currentY, volW, STEP_H };
        d.filledRect({ trk.volRect.left, trk.volRect.top }, { volW, STEP_H }, { .color = { 40, 40, 45 } });
        d.filledRect({ trk.volRect.left, trk.volRect.top + STEP_H / 2 - 2 }, { (int)(volW * trk.volume), 4 }, { .color = trk.themeColor });

        trk.genRect = { trk.volRect.left + volW + sp, currentY, genW, STEP_H };
        trk.lenBtnRect = trk.genRect;
        d.filledRect({ trk.genRect.left, trk.genRect.top }, { genW, STEP_H }, { .color = { 60, 60, 75 } });
        d.textCentered({ trk.genRect.left + genW / 2, trk.genRect.top + 1 }, "G" + std::to_string(trk.genLen), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        int gx = trk.genRect.left + genW + 10;
        for (int s = 0; s < SEQ_STEPS; s++)
            trk.stepRects[s] = { gx + s * stepW, currentY, stepW - 1, STEP_H / 2 + LANE_H };
        currentY += 26;
    }
    
    return currentY;
}

int drawSeqUI(Draw& d, sf::Vector2u size, int currentY)
{
    if (studio.selTrack != -1 && studio.selStep != -1) {
        auto& s = studio.tracks[studio.selTrack]->sequence[studio.selStep];
        int eY = currentY;
        d.text({ MARGIN, eY }, "EDIT T" + std::to_string(studio.selTrack + 1) + " S" + std::to_string(studio.selStep + 1), 8,
            { .color = studio.tracks[studio.selTrack]->themeColor, .font = &PoppinsLight_8 });
        studio.editNoteRect = { 100, eY - 2, 80, 15 };
        d.text({ 100, eY }, "NOTE: " + std::string(MIDI_NOTES_STR[s.note]), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
        studio.editLenRect = { 200, eY - 2, 80, 15 };
        d.text({ 200, eY }, "LEN: " + fToString(s.len, 1) + "steps", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
        studio.editVeloRect = { 300, eY - 2, 80, 15 };
        d.text({ 300, eY }, "VEL: " + std::to_string((int)(s.velocity * 100)) + "%", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
        studio.editProbRect = { 400, eY - 2, 80, 15 };
        d.text({ 400, eY }, "PROB: " + std::to_string((int)(s.condition * 100)) + "%", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
        sf::IntRect* tr = nullptr;
        switch (studio.stepEditMode) {
        case EDIT_NOTE:
            tr = &studio.editNoteRect;
            break;
        case EDIT_VELO:
            tr = &studio.editVeloRect;
            break;
        case EDIT_PROB:
            tr = &studio.editProbRect;
            break;
        case EDIT_LEN:
            tr = &studio.editLenRect;
            break;
        }
        if (tr) d.filledRect({ tr->left, tr->top + tr->height }, { tr->width, 2 },
            { .color = studio.tracks[studio.selTrack]->themeColor });
        currentY += 16;
    }

    int sw = studio.tracks[0]->stepRects[0].width + 1;
    for (int t = 0; t < MAX_TRACKS; t++) {
        auto& trk = studio.tracks[t];
        for (int s = 0; s < SEQ_STEPS; s++) {
            auto& r = trk->stepRects[s];
            auto& step = trk->sequence[s];
            if (step.active) {
                float nm = 1.f - (float)(CLAMP(step.note, 24, 96) - 24) / 72.f;
                int ny = r.top + r.height - LANE_H + 1 + (int)(nm * LANE_H);
                d.line({ r.left, ny }, { r.left + (int)(step.len * (sw - 1)) - 1, ny }, { .color = trk->themeColor });
            }
        }
    }

    return currentY;
}

void drawStepEditUI(Draw& d, sf::Vector2u size, int currentY)
{
}

void updateSequencerPixels(std::vector<sf::Uint8>& pixels, int stride)
{
    int sw = studio.tracks[0]->stepRects[0].width + 1;
    int ph = (int)((studio.currentStep + studio.sampleCounter.load() / studio.samplesPerStep) * sw);
    int gx = studio.tracks[0]->stepRects[0].left;
    for (int t = 0; t < MAX_TRACKS; t++) {
        auto& trk = studio.tracks[t];
        for (int s = 0; s < SEQ_STEPS; s++) {
            auto& r = trk->stepRects[s];
            int h = r.height - LANE_H;
            Color c = trk->sequence[s].active ? trk->themeColor : (s % 4 == 0 ? Color { 55, 55, 60 } : Color { 45, 45, 50 });
            int selY = (studio.selTrack == t && studio.selStep == s) ? h - 3 : h;
            for (int y = 0; y < h; y++)
                for (int x = 0; x < r.width; x++) {
                    int gX = r.left + x;
                    size_t idx = ((r.top + y) * stride + gX) * 4;
                    if (studio.isPlaying && (gX == gx + ph || gX == gx + ph - 1))
                        pixels[idx] = pixels[idx + 1] = pixels[idx + 2] = 255;
                    else if (y >= selY)
                        pixels[idx] = pixels[idx + 1] = pixels[idx + 2] = 255;
                    else {
                        pixels[idx] = c.r;
                        pixels[idx + 1] = c.g;
                        pixels[idx + 2] = c.b;
                    }
                }
        }
    }
}
