#pragma once

#include "zic23/studio.h"
#include "helpers/clamp.h"

void stretchTrackSequence(Track& trk, bool setLen = false)
{
    if (trk.seqDisplayLen <= 4) return;
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
    if (setLen) trk.seqDisplayLen /= 2;
}

void compressTrackSequence(Track& trk, bool setLen = false)
{
    if (trk.seqDisplayLen >= 128) return;
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
    if (setLen) trk.seqDisplayLen *= 2;
}

void runGeneration(int trkIdx)
{
    Track& trk = *studio.tracks[trkIdx];
    if (trk.generate != nullptr) {
        trk.generate(trk.sequence);
        // Apply stretch logic based on display state (64 is native)
        if (trk.seqDisplayLen == 32) stretchTrackSequence(trk);
        else if (trk.seqDisplayLen == 16) {
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
        } else if (trk.seqDisplayLen == 8) {
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
        } else if (trk.seqDisplayLen == 4) {
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
        } else if (trk.seqDisplayLen == 128) {
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
