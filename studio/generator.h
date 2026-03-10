#pragma once

#include <random>
#include <vector>

#include "studio/step.h"

// --- GENERATOR LOGIC ---
namespace Generator {
static std::random_device rd;
static std::mt19937 gen(rd());

float rand01()
{
    return std::uniform_real_distribution<float>(0.0f, 1.0f)(gen);
}

void clearSequence(std::vector<Step>& sequence)
{
    for (int i = 0; i < SEQ_STEPS; i++)
        sequence[i].active = false;
}

void generateKick(std::vector<Step>& sequence)
{
    clearSequence(sequence);

    bool lastHasGhost = false;
    for (int i = 0; i < SEQ_STEPS; i += 4) {
        // Main Tribe Kick
        sequence[i].active = true;
        sequence[i].velocity = 0.9f + rand01() * 0.1f;
        sequence[i].note = 60;
        sequence[i].condition = 1.0f;

        lastHasGhost = false;
        // Ghost/Rumble logic for Mental
        for (int j = 1; j < 4; j++) {
            if (i + j < SEQ_STEPS && rand01() < 0.18f) {
                sequence[i + j].active = true;
                sequence[i + j].velocity = 0.35f + (rand01() * 0.2f);
                sequence[i + j].note = 60;
                sequence[i + j].condition = 1.0f;
                lastHasGhost = true;
            }
        }
    }
}

void generatePerc(std::vector<Step>& sequence, bool isClap = false)
{
    clearSequence(sequence);

    int i = rand01() < 0.5f && !isClap ? 2 : (rand01() < 0.5f ? 0 : 4);
    int inc = i == 2 ? 4 : 8;
    float flamChance = isClap ? 0.1f : 0.2f;
    for (; i < SEQ_STEPS; i += inc) {

        float vel = 0.7f + rand01() * 0.3f;

        sequence[i].active = true;
        sequence[i].velocity = vel;
        sequence[i].note = 60;
        sequence[i].condition = 1.0f;

        // flam
        if (rand01() < flamChance && i + 1 < SEQ_STEPS) {
            sequence[i + 1].active = true;
            sequence[i + 1].velocity = vel * 0.5f;
            sequence[i + 1].note = 60;
            sequence[i + 1].condition = 1.0f;
        }
    }
}

void generateSnareHat(std::vector<Step>& sequence)
{
    generatePerc(sequence);
}

void generateClap(std::vector<Step>& sequence)
{
    generatePerc(sequence, true);
}
}
