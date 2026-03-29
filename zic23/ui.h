#pragma once

#include "zic23/studio.h"
#include "helpers/clamp.h"

void stretchTrackSequence(Track& trk, bool setLen = false)
{
    if (trk.genLen <= 4) return;
    std::vector<Step> newSeq(SEQ_STEPS);
    for (int i = 0; i < 32; i++) {
        if (trk.sequence[i].active) {
            newSeq[i * 2] = trk.sequence[i];
            if (trk.type == TRACK_TYPE_SYNTH) {
                newSeq[i * 2].len *= 2.0f;
            }
        }
    }
    for (int i = 0; i < SEQ_STEPS; i++)
        trk.sequence[i] = newSeq[i];
    if (setLen) trk.genLen /= 2;
}

void compressTrackSequence(Track& trk, bool setLen = false)
{
    if (trk.genLen >= 128) return;
    std::vector<Step> newSeq(SEQ_STEPS);
    for (int i = 0; i < SEQ_STEPS; i++) {
        if (trk.sequence[i].active) {
            int newIdx = i / 2;
            newSeq[newIdx] = trk.sequence[i];
            newSeq[newIdx].active = true;
            if (trk.type == TRACK_TYPE_SYNTH) {
                newSeq[newIdx].len = std::max(0.5f, newSeq[newIdx].len / 2.0f);
            }
        }
    }
    for (int i = 0; i < SEQ_STEPS; i++) {
        if (i > 31) trk.sequence[i] = newSeq[i - 32];
        else trk.sequence[i] = newSeq[i];
    }
    if (setLen) trk.genLen *= 2;
}

void runGeneration(int trkIdx)
{
    Track& trk = *studio.tracks[trkIdx];
    if (trk.generate != nullptr) {
        trk.generate(trk.sequence);
        // Apply stretch logic based on display state (64 is native)
        if (trk.genLen == 32) stretchTrackSequence(trk);
        else if (trk.genLen == 16) {
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
        } else if (trk.genLen == 8) {
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
        } else if (trk.genLen == 4) {
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
        } else if (trk.genLen == 128) {
            compressTrackSequence(trk);
        }

        studio.selTrack = trkIdx;
        studio.selStep = 0;
    }
}

void editStep(Step& step, StepEditMode mode, int scaled)
{
    if (mode == StepEditMode::EDIT_NOTE) step.note = CLAMP(step.note + scaled, 0, (int)MIDI_LAST_NOTE);
    else if (mode == StepEditMode::EDIT_VELO) step.velocity = CLAMP(step.velocity + (scaled * 0.05f), 0.0f, 1.0f);
    else if (mode == StepEditMode::EDIT_PROB) step.condition = CLAMP(step.condition + (scaled * 0.01f), 0.0f, 1.0f);
    else if (mode == StepEditMode::EDIT_LEN) step.len = CLAMP(step.len + (scaled * 0.5f), 0.5f, 64.5f);
}

void duplicateTrackSequence(Track& trk)
{
    int lastActive = -1;
    for (int i = 0; i < SEQ_STEPS; i++) {
        if (trk.sequence[i].active) lastActive = i;
    }

    if (lastActive == -1) return;

    int currentLen = 0;
    if (lastActive < 4) currentLen = 4;
    else if (lastActive < 8) currentLen = 8;
    else if (lastActive < 16) currentLen = 16;
    else if (lastActive < 32) currentLen = 32;
    else return; // Already at max or filled beyond 32

    for (int i = 0; i < currentLen; i++) {
        if (currentLen + i < SEQ_STEPS) {
            trk.sequence[currentLen + i] = trk.sequence[i];
        }
    }
}

void deleteTrackSequence(Track& trk)
{
    int lastActive = -1;
    for (int i = 0; i < SEQ_STEPS; i++) {
        if (trk.sequence[i].active) lastActive = i;
    }

    if (lastActive == -1) return;

    int i = 0;
    if (lastActive < 4) i = 0;
    else if (lastActive < 8) i = 4;
    else if (lastActive < 16) i = 8;
    else if (lastActive < 32) i = 16;
    else i = 32;

    for (; i < SEQ_STEPS; i++) {
        trk.sequence[i].active = false;
    }
}


void drawTopBarUI(Draw& d, sf::Vector2u size)
{
    const int winW = (int)size.x;
    d.filledRect({ 0, 0 }, { winW, 25 }, { .color = d.styles.colors.quaternary });

    studio.transportRect = { MARGIN, 4, 60, 17 };
    d.filledRect({ MARGIN, 4 }, { 60, 17 }, { .color = studio.isPlaying ? Color { 200, 50, 50 } : Color { 50, 200, 50 } });
    d.text({ MARGIN + 6, 7 }, studio.isPlaying ? "STOP" : "PLAY", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    helpBtnRect = { MARGIN + 70, 4, 60, 17 };
    d.filledRect({ helpBtnRect.left, helpBtnRect.top }, { 60, 17 }, { .color = { 60, 60, 75 } });
    d.text({ helpBtnRect.left + 14, 7 }, "HELP", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    std::stringstream bss;
    bss << "BPM: " << std::fixed << std::setprecision(1) << studio.bpm.load();
    studio.bpmRect = { winW - 100, 0, 90, 25 };
    d.textRight({ winW - MARGIN, 6 }, bss.str(), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
}
