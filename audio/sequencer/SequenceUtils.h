#pragma once

#include <algorithm>
#include <vector>

#include "audio/sequencer/Step.h"

inline void stretchSequence(std::vector<Step>& sequence, uint32_t& genLen, bool isSynth, bool setLen = true)
{
    if (genLen <= 4) return;
    std::vector<Step> newSeq(SEQ_STEPS);
    for (int i = 0; i < 32; i++) {
        if (sequence[i].active) {
            newSeq[i * 2] = sequence[i];
            if (isSynth) {
                newSeq[i * 2].len *= 2.0f;
            }
        }
    }
    for (int i = 0; i < SEQ_STEPS; i++) {
        sequence[i] = newSeq[i];
    }
    if (setLen) genLen /= 2;
}

inline void compressSequence(std::vector<Step>& sequence, uint32_t& genLen, bool isSynth, bool setLen = true)
{
    if (genLen >= 128) return;
    std::vector<Step> newSeq(SEQ_STEPS);
    for (int i = 0; i < SEQ_STEPS; i++) {
        if (sequence[i].active) {
            int newIdx = i / 2;
            newSeq[newIdx] = sequence[i];
            newSeq[newIdx].active = true;
            if (isSynth) {
                newSeq[newIdx].len = std::max(0.5f, newSeq[newIdx].len / 2.0f);
            }
        }
    }
    for (int i = 0; i < SEQ_STEPS; i++) {
        if (i > 31) sequence[i] = newSeq[i - 32];
        else sequence[i] = newSeq[i];
    }
    if (setLen) genLen *= 2;
}
