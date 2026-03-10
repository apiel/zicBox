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

void generatePerc(std::vector<Step>& sequence, int start, int inc, float flamChance = 0.0f, float flamChance2 = 0.0f, float halfGhostChance = 0.0f)
{
    clearSequence(sequence);

    for (int i = start; i < SEQ_STEPS; i += inc) {

        float vel = 0.7f + rand01() * 0.3f;

        sequence[i].active = true;
        sequence[i].velocity = vel;
        sequence[i].note = 60;
        sequence[i].condition = 1.0f;

        if (flamChance) {
            if (rand01() < flamChance && i + 1 < SEQ_STEPS) {
                sequence[i + 1].active = true;
                sequence[i + 1].velocity = vel * 0.5f;
                sequence[i + 1].note = 60;
                sequence[i + 1].condition = 1.0f;

                if (rand01() < flamChance2 && i + 2 < SEQ_STEPS) {
                    sequence[i + 2].active = true;
                    sequence[i + 2].velocity = vel * 0.5f;
                    sequence[i + 2].note = 60;
                    sequence[i + 2].condition = 1.0f;
                }
            }
        }
        if (rand01() < halfGhostChance) {
            int halfInc = inc / 2;
            if (i + halfInc < SEQ_STEPS) {
                sequence[i + halfInc].active = true;
                sequence[i + halfInc].velocity = vel * 0.5f;
                sequence[i + halfInc].note = 60;
                sequence[i + halfInc].condition = 1.0f;
            }
        }
    }
}

void generateSnare(std::vector<Step>& sequence)
{
    int start = rand01() < 0.9f ? 2 : 4;
    int inc = start == 2 ? (rand01() < 0.8f ? 4 : 8) : 8;
    float flamChance = 0.2f;
    float flamChance2 = 0.2f;
    generatePerc(sequence, start, inc, flamChance, flamChance2);
}

void generateHat(std::vector<Step>& sequence)
{
    int start = rand01() < 0.8f ? 0 : 2;
    int inc = rand01() < 0.8f ? 8 : 4;
    float flamChance = 0.1f;
    float flamChance2 = 0.05f;
    float halfGhostChance = 0.1f;
    generatePerc(sequence, start, inc, flamChance, flamChance2, halfGhostChance);
}

void generateClap(std::vector<Step>& sequence)
{
    int start = rand01() < 0.8f ? 4 : 0;
    int inc = rand01() < 0.8f ? 8 : 4;
    float flamChance = 0.1f;
    float flamChance2 = 0.05f;
    float halfGhostChance = 0.1f;
    generatePerc(sequence, start, inc, flamChance, flamChance2, halfGhostChance);
}
}
