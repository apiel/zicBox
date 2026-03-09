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

void generateKick(std::vector<Step>& sequence)
{
    bool lastHasGhost = false;
    for (int i = 0; i < SEQ_STEPS; i++)
        sequence[i].active = false;

    for (int i = 0; i < SEQ_STEPS; i += 4) {
        // Main Tribe Kick
        sequence[i].active = true;
        sequence[i].velocity = 0.9f + rand01() * 0.1f;
        sequence[i].note = 36;
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

void generateSnareHat(std::vector<Step>& sequence)
{
    for (int i = 0; i < SEQ_STEPS; i++)
        sequence[i].active = false;

    float density = 0.55f;

    for (int i = 0; i < SEQ_STEPS; i++) {
        bool isOffbeat = (i % 4 == 2);
        float prob = isOffbeat ? density : density * 0.15f;

        if (rand01() < prob) {
            sequence[i].active = true;
            sequence[i].velocity = isOffbeat ? 0.85f : 0.45f;
            sequence[i].note = 60;
            sequence[i].condition = 1.0f;

            // Mental Roll/Flam
            if (rand01() < 0.12f && i < SEQ_STEPS - 1) {
                sequence[i + 1].active = true;
                sequence[i + 1].velocity = sequence[i].velocity * 0.5f;
                sequence[i + 1].note = sequence[i].note;
                sequence[i + 1].condition = 1.0f;
            }
        }
    }
}

}