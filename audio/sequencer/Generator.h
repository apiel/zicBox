#pragma once

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

#include "audio/sequencer/Step.h"

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

inline void generateKick(std::vector<Step>& sequence, float p1, float p2, float p3)
{
    clearSequence(sequence);

    // p1: Tribe kick velocity amount
    float baseVelocity = 0.4f + p1 * 0.55f; // range 0.4 .. 0.95

    // p2: Ghost note density
    float baseGhostChance = p2 * 0.36f; // range 0% .. 36% (default 0.18f)

    // p3: End-of-loop rumble probability boost
    float endRumbleBoost = p3 * 0.4f;

    for (int i = 0; i < SEQ_STEPS; i += 4) {
        // Main Tribe Kick
        sequence[i].active = true;
        sequence[i].velocity = baseVelocity + rand01() * 0.05f;
        sequence[i].note = 60;
        sequence[i].condition = 1.0f;

        // Ghost/Rumble logic for Mental
        for (int j = 1; j < 4; j++) {
            if (i + j < SEQ_STEPS) {
                float chance = baseGhostChance;
                if (i >= SEQ_STEPS - 4) { // last beat boost
                    chance += endRumbleBoost;
                }
                if (rand01() < chance) {
                    sequence[i + j].active = true;
                    sequence[i + j].velocity = 0.35f + (rand01() * 0.2f);
                    sequence[i + j].note = j > 1 && rand01() < 0.1f ? 72 : 60;
                    sequence[i + j].condition = 1.0f;
                }
            }
        }
    }
}

inline void generateKick(std::vector<Step>& sequence)
{
    generateKick(sequence, 0.5f, 0.5f, 0.5f);
}

inline void generateBass(std::vector<Step>& sequence, float p1, float p2, float p3)
{
    clearSequence(sequence);

    // ── Tonality ─────────────────────────────────────────
    struct NW {
        int semi;
        float w;
    };
    auto pickNote = [](const std::vector<NW>& t) -> int {
        float total = 0.f;
        for (auto& n : t)
            total += n.w;
        float r = rand01() * total;
        for (auto& n : t) {
            r -= n.w;
            if (r <= 0.f) return n.semi;
        }
        return t.back().semi;
    };

    const int ROOT = 48; // C3

    std::vector<NW> loPool;
    std::vector<NW> hiPool;

    // p1: Scale Selection
    if (p1 < 0.25f) { // Minor Pentatonic
        loPool = { { 0, 6.0f }, { 3, 3.0f }, { 5, 2.0f }, { 7, 3.0f }, { 10, 2.0f } };
        hiPool = { { 12, 5.0f }, { 15, 3.0f }, { 17, 2.0f }, { 19, 3.0f }, { 22, 2.0f } };
    } else if (p1 < 0.5f) { // Natural Minor
        loPool = { { 0, 6.0f }, { 2, 1.0f }, { 3, 2.5f }, { 7, 3.0f }, { 8, 1.0f }, { 10, 1.5f } };
        hiPool = { { 12, 5.0f }, { 14, 1.0f }, { 15, 2.5f }, { 19, 3.0f }, { 20, 1.0f }, { 22, 1.5f } };
    } else if (p1 < 0.75f) { // Dorian
        loPool = { { 0, 6.0f }, { 2, 1.0f }, { 3, 2.5f }, { 7, 3.0f }, { 9, 1.5f }, { 10, 1.5f } };
        hiPool = { { 12, 5.0f }, { 14, 1.0f }, { 15, 2.5f }, { 19, 3.0f }, { 21, 1.5f }, { 22, 1.5f } };
    } else { // Acid Chromatic / original
        loPool = { { 0, 6.0f }, { 3, 2.0f }, { 7, 3.0f }, { 10, 1.5f }, { 2, 0.7f } };
        hiPool = { { 12, 5.0f }, { 15, 2.0f }, { 19, 1.5f }, { 17, 0.8f }, { 14, 0.5f } };
    }

    // p2: Rhythm Density
    int minSteps = 4;
    int maxSteps = 15;
    int density = minSteps + (int)(p2 * (maxSteps - minSteps));

    std::vector<bool> grid(16, false);
    grid[0] = true;
    if (rand01() < 0.85f) grid[8] = true;

    std::vector<int> candidates = { 2, 3, 4, 5, 6, 9, 10, 11, 13, 14, 15, 1, 7, 12 };
    int placed = 2;
    for (int idx : candidates) {
        if (placed >= density) break;
        float chance = (idx % 2 == 1) ? 0.75f : 0.55f;
        if (rand01() < chance) {
            grid[idx] = true;
            placed++;
        }
    }

    struct MotifStep {
        int note;
        float vel;
        int len;
        bool slide;
    };
    std::vector<MotifStep> motif(16, { 0, 0.f, 1, false });

    int lastActiveIdx = -1;
    int lastNote = ROOT;

    float octaveJumpChance = 0.15f + p3 * 0.4f;
    float slideChance = 0.2f + p3 * 0.7f;
    float stepSlideChance = 0.1f + p3 * 0.3f;

    for (int i = 0; i < 16; i++) {
        if (!grid[i]) continue;

        bool goHigh = (rand01() < octaveJumpChance);
        int note = ROOT + (goHigh ? pickNote(hiPool) : pickNote(loPool));

        bool isDownbeat = (i == 0 || i == 8);
        bool isSurprise = (!isDownbeat && rand01() < 0.15f);
        float vel;
        if (isDownbeat) vel = 0.80f + rand01() * 0.17f;
        else if (isSurprise) vel = 0.75f + rand01() * 0.15f;
        else vel = 0.45f + rand01() * 0.25f;

        int len;
        float lr = rand01();
        if (lr < 0.55f) len = 1;
        else if (lr < 0.80f) len = 2;
        else if (lr < 0.93f) len = 3;
        else len = 4;

        motif[i] = { note, vel, len, false };

        if (lastActiveIdx >= 0) {
            int interval = std::abs(note - lastNote);
            bool longPrev = (motif[lastActiveIdx].len >= 2);
            bool stepwise = (interval <= 5);
            bool bigJump = (interval >= 12);
            if (longPrev && (stepwise || bigJump) && rand01() < slideChance)
                motif[lastActiveIdx].slide = true;
            else if (!longPrev && stepwise && rand01() < stepSlideChance)
                motif[lastActiveIdx].slide = true;
        }

        lastNote = note;
        lastActiveIdx = i;
    }

    if (lastActiveIdx >= 0 && lastActiveIdx > 8) {
        bool toFifth = (rand01() < 0.5f);
        motif[lastActiveIdx].note = ROOT + (toFifth ? 7 : 0);
        motif[lastActiveIdx].vel = 0.88f;
        motif[lastActiveIdx].slide = false;

        for (int j = lastActiveIdx - 1; j >= 0; j--) {
            if (grid[j]) {
                motif[j].slide = true;
                motif[j].note = motif[lastActiveIdx].note - 1;
                break;
            }
        }
    }

    bool do32 = (rand01() < 0.40f);

    for (int i = 0; i < SEQ_STEPS; i++) {
        int pos16 = i % 16;
        bool inVar = do32 && ((i % 32) >= 16);

        if (!grid[pos16]) continue;
        MotifStep m = motif[pos16];

        if (inVar) {
            float vr = rand01();
            if (vr < 0.18f) continue;
            else if (vr < 0.35f) m.note += 12;
            else if (vr < 0.45f) m.note -= 12;
            else if (vr < 0.55f) m.len = std::min(m.len + 1, 4);
            m.note = std::max(24, std::min(m.note, 84));
        }

        sequence[i].active = true;
        sequence[i].note = (uint8_t)m.note;
        sequence[i].velocity = m.vel;
        sequence[i].len = m.len + (m.slide ? 0.5f : 0.0f);
    }
}

inline void generateBass(std::vector<Step>& sequence)
{
    generateBass(sequence, 0.5f, 0.5f, 0.5f);
}

inline void generateDrum(std::vector<Step>& sequence, float p1, float p2, float p3)
{
    clearSequence(sequence);

    int start = 0;
    int inc = 4;
    float flamChance = p2 * 0.4f;
    float flamChance2 = p2 * 0.3f;
    float halfGhostChance = p2 * 0.3f;

    if (p3 < 0.3f) {
        inc = 8;
    } else if (p3 < 0.7f) {
        inc = 4;
    } else {
        inc = 2;
    }

    if (p1 < 0.2f) { // Snare
        start = rand01() < 0.9f ? 2 : 4;
        if (inc == 8) start = 4;
    } else if (p1 < 0.4f) { // Hat
        start = rand01() < 0.8f ? 0 : 2;
    } else if (p1 < 0.6f) { // Clap
        start = rand01() < 0.8f ? 4 : 0;
    } else if (p1 < 0.8f) { // Perc
        start = rand01() < 0.5f ? 0 : 2;
    } else { // Mixed
        start = randInt(0, 3);
    }

    for (int i = start; i < SEQ_STEPS; i += inc) {
        float vel = 0.7f + rand01() * 0.3f;

        sequence[i].active = true;
        sequence[i].velocity = vel;
        sequence[i].note = 60;
        sequence[i].condition = 1.0f;

        if (flamChance > 0.0f) {
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
        if (rand01() < halfGhostChance && inc > 1) {
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

inline void generateDrum(std::vector<Step>& sequence)
{
    generateDrum(sequence, 0.5f, 0.5f, 0.5f);
}

// ── Legacy / Discrete Generators (used by zic23 & zicRack) ────────────────

inline void generatePerc(std::vector<Step>& sequence, int start, int inc, float flamChance = 0.0f, float flamChance2 = 0.0f, float halfGhostChance = 0.0f)
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
        if (rand01() < halfGhostChance && inc > 1) {
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

inline void generateSnare(std::vector<Step>& sequence)
{
    int start = rand01() < 0.9f ? 2 : 4;
    int inc = start == 2 ? (rand01() < 0.8f ? 4 : 8) : 8;
    float flamChance = 0.2f;
    float flamChance2 = 0.2f;
    generatePerc(sequence, start, inc, flamChance, flamChance2);
}

inline void generateHat(std::vector<Step>& sequence)
{
    int start = rand01() < 0.8f ? 0 : 2;
    int inc = rand01() < 0.8f ? 8 : 4;
    float flamChance = 0.1f;
    float flamChance2 = 0.05f;
    float halfGhostChance = 0.1f;
    generatePerc(sequence, start, inc, flamChance, flamChance2, halfGhostChance);
}

inline void generateClap(std::vector<Step>& sequence)
{
    int start = rand01() < 0.8f ? 4 : 0;
    int inc = rand01() < 0.8f ? 8 : 4;
    float flamChance = 0.1f;
    float flamChance2 = 0.05f;
    float halfGhostChance = 0.1f;
    generatePerc(sequence, start, inc, flamChance, flamChance2, halfGhostChance);
}

} // namespace Generator
