#pragma once

#include <random>
#include <vector>
#include "zicGridV2/step.h"

namespace Generator {
inline static std::random_device rd;
inline static std::mt19937 gen(rd());

inline float rand01()
{
    return std::uniform_real_distribution<float>(0.0f, 1.0f)(gen);
}

inline int randInt(int min, int max)
{
    return std::uniform_int_distribution<int>(min, max)(gen);
}

inline void clearSequence(std::vector<Step>& sequence)
{
    for (int i = 0; i < SEQ_STEPS; i++) {
        sequence[i].active = false;
    }
}

inline void generateKick(std::vector<Step>& sequence, float p1 = 0.5f, float p2 = 0.5f, float p3 = 0.5f)
{
    clearSequence(sequence);
    float baseVelocity = 0.4f + p1 * 0.55f;
    for (int i = 0; i < SEQ_STEPS; i += 4) {
        sequence[i].active = true;
        sequence[i].velocity = baseVelocity;
        sequence[i].note = 60;
        sequence[i].condition = 1.0f;
    }
}

inline void generateBass(std::vector<Step>& sequence, float p1 = 0.5f, float p2 = 0.5f, float p3 = 0.5f)
{
    clearSequence(sequence);
    int baseNote = 36;
    for (int i = 0; i < SEQ_STEPS; i += 2) {
        if (rand01() < p1) {
            sequence[i].active = true;
            sequence[i].note = baseNote + randInt(0, 12);
            sequence[i].velocity = 0.7f + p2 * 0.25f;
            sequence[i].len = (rand01() < p3) ? 1.5f : 0.8f;
        }
    }
}
}
