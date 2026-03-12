#pragma once

#include <random>
#include <vector>

#include "tek23/step.h"

// --- GENERATOR LOGIC ---
namespace Generator {
static std::random_device rd;
static std::mt19937 gen(rd());

float rand01()
{
    return std::uniform_real_distribution<float>(0.0f, 1.0f)(gen);
}

int randInt(int min, int max)
{
    return std::uniform_int_distribution<int>(min, max)(gen);
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
                sequence[i + j].note = j > 1 && rand01() < 0.1f ? 72 : 60;
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

void generateBass(std::vector<Step>& sequence)
{
    clearSequence(sequence);

    int patternLen = 16; // base beat length
    int melodyLen = rand01() < 0.5f ? 16 : 32;
    int octaveOffset = 12; // melody octave
    int beatNote = 60; // main note (C)
    int beatAlt = beatNote + ((rand01() < 0.4f) ? 1 : 0); // optional sharp note

    std::vector<int> beatPattern(patternLen, -1); // -1 = rest

    // --- 1. Create the beat ---
    for (int i = 0; i < patternLen; i++) {
        if (rand01() < 0.5f) { // ~50% chance to have a note
            beatPattern[i] = (rand01() < 0.5f) ? beatAlt : beatNote;
        }
    }

    // --- 2. Fill the melody (above) ---
    std::vector<int> melody(melodyLen, -1); // -1 = skip
    for (int i = 0; i < melodyLen; i++) {
        // if (beatPattern[i % patternLen] < 0 && rand01() < 0.5f) {
        // instead at the beginning give less chance, and more chance at the end
        if (beatPattern[i % patternLen] < 0 && rand01() < (i < melodyLen / 2 ? 0.1f : 0.5f)) {
            melody[i] = beatNote + 10 + randInt(0, 4); // +12 (octave) +2 -2
        }
    }

    Step* lastMelodyStep = nullptr;
    // --- 3. Push to steps vector for the full pattern ---
    for (int i = 0; i < SEQ_STEPS; i++) {
        int posInPattern = i % patternLen;

        // Beat note
        if (beatPattern[posInPattern] >= 0) {
            sequence[i].active = true;
            sequence[i].velocity = 0.7f;
            // TODO impl len
            // sequence[i].len = (uint16_t)(1 + ((rand01() < 0.1f) ? 2 : 1)); // rare double length
            sequence[i].note = (uint8_t)beatPattern[posInPattern];
        } else {
            int posInMelody = i % melodyLen;
            if (melody[posInMelody] >= 0) {
                // steps.push_back({ .enabled = true,
                //     .velocity = 1.0f,
                //     .position = (uint16_t)i,
                //     .len = (uint16_t)(1 + ((rand01() < 0.1f) ? 2 : 1)), // rare double length
                //     .note = (uint8_t)melody[posInMelody] });

                sequence[i].active = true;
                sequence[i].velocity = 0.7f;
                // TODO impl len
                // sequence[i].len = (uint16_t)(1 + ((rand01() < 0.1f) ? 2 : 1)); // rare double length
                sequence[i].note = (uint8_t)melody[posInMelody];
                lastMelodyStep = &sequence[i];
            }
        }
    }

    // Optional last twist: change last melody note
    if (lastMelodyStep && rand01() < 0.3f) {
        // melodyNotes[melodySteps - 1] = beatNote + octaveOffset + 5 + randInt(genSeed, 0, 3);
        lastMelodyStep->note += randInt(1, 5);
    }
}
}
