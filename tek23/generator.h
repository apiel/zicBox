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

// void generateBass(std::vector<Step>& sequence)
// {
//     clearSequence(sequence);

//     int patternLen = 16; // base beat length
//     int melodyLen = rand01() < 0.5f ? 16 : 32;
//     int octaveOffset = 12; // melody octave
//     int beatNote = 60; // main note (C)
//     int beatAlt = beatNote + ((rand01() < 0.4f) ? 1 : 0); // optional sharp note

//     std::vector<int> beatPattern(patternLen, -1); // -1 = rest

//     // --- 1. Create the beat ---
//     for (int i = 0; i < patternLen; i++) {
//         if (rand01() < 0.5f) { // ~50% chance to have a note
//             beatPattern[i] = (rand01() < 0.5f) ? beatAlt : beatNote;
//         }
//     }

//     std::vector<int> melody(melodyLen, -1); // -1 = skip
//     for (int i = 0; i < melodyLen; i++) {
//         if (beatPattern[i % patternLen] < 0 && rand01() < (i < melodyLen / 2 ? 0.1f : 0.5f)) {
//             melody[i] = beatNote + 10 + randInt(0, 4); // +12 (octave) +2 -2
//         }
//     }

//     Step* lastMelodyStep = nullptr;
//     // --- 3. Push to steps vector for the full pattern ---
//     for (int i = 0; i < SEQ_STEPS; i++) {
//         int posInPattern = i % patternLen;

//         // Beat note
//         if (beatPattern[posInPattern] >= 0) {
//             sequence[i].active = true;
//             sequence[i].velocity = 0.7f;
//             sequence[i].len = (1 + ((rand01() < 0.1f) ? 2 : 1));
//             sequence[i].note = (uint8_t)beatPattern[posInPattern];
//         } else {
//             int posInMelody = i % melodyLen;
//             if (melody[posInMelody] >= 0) {
//                 sequence[i].active = true;
//                 sequence[i].velocity = 0.7f;
//                 sequence[i].len = (1 + ((rand01() < 0.1f) ? 2 : 1));
//                 sequence[i].note = (uint8_t)melody[posInMelody];
//                 lastMelodyStep = &sequence[i];
//             }
//         }
//     }

//     // Optional last twist: change last melody note
//     if (lastMelodyStep && rand01() < 0.3f) {
//         // melodyNotes[melodySteps - 1] = beatNote + octaveOffset + 5 + randInt(genSeed, 0, 3);
//         lastMelodyStep->note += randInt(1, 5);
//     }
// }

void generateBass(std::vector<Step>& sequence)
{
    clearSequence(sequence);

    // ── Tonality ─────────────────────────────────────────
    // Minor scale semitone offsets with weights.
    // Root/oct are heaviest — acid lives on those two.
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

    // Two registers: low (bass punch) and high (acid scream)
    std::vector<NW> loPool = {
        { 0, 6.0f }, // root
        { 3, 2.0f }, // m3
        { 7, 3.0f }, // 5th
        { 10, 1.5f }, // m7
        { 2, 0.7f }, // 2nd (passing)
    };
    std::vector<NW> hiPool = {
        { 12, 5.0f }, // octave      ← most important acid jump
        { 15, 2.0f }, // oct + m3
        { 19, 1.5f }, // oct + 5th
        { 17, 0.8f }, // oct + m6    (mental/mystical color)
        { 14, 0.5f }, // oct + M2
    };

    int density = 8 + randInt(0, 5); // 8..12 active steps out of 16

    // Build a rhythm grid: place notes rhythmically
    // Core rule: beat 1 (step 0) always ON, beat 3 (step 8) usually ON
    std::vector<bool> grid(16, false);
    grid[0] = true;
    if (rand01() < 0.85f) grid[8] = true;

    // Fill remaining slots. Prefer off-beats and 16th positions
    // that create syncopation: steps 2,3,5,6,9,10,11,13,14
    std::vector<int> candidates = { 2, 3, 4, 5, 6, 9, 10, 11, 13, 14, 15, 1, 7, 12 };
    int placed = 2; // already placed 0 and 8
    for (int idx : candidates) {
        if (placed >= density) break;
        // Higher chance for "acid-typical" positions
        float chance = (idx % 2 == 1) ? 0.75f : 0.55f; // odd steps = off-beat
        if (rand01() < chance) {
            grid[idx] = true;
            placed++;
        }
    }

    // Build 16-step motif
    struct MotifStep {
        int note;
        float vel;
        int len;
        bool slide;
    };
    std::vector<MotifStep> motif(16, { 0, 0.f, 1, false });

    int lastActiveIdx = -1;
    int lastNote = ROOT;

    for (int i = 0; i < 16; i++) {
        if (!grid[i]) continue;

        // Note: mix low and high register — octave jumps ~35%
        bool goHigh = (rand01() < 0.35f);
        int note = ROOT + (goHigh ? pickNote(hiPool) : pickNote(loPool));

        // Velocity: accent on beat 1 (i==0), beat 3 (i==8),
        // and random "surprise accents" ~15% of other steps
        bool isDownbeat = (i == 0 || i == 8);
        bool isSurprise = (!isDownbeat && rand01() < 0.15f);
        float vel;
        if (isDownbeat) vel = 0.80f + rand01() * 0.17f;
        else if (isSurprise) vel = 0.75f + rand01() * 0.15f;
        else vel = 0.45f + rand01() * 0.25f;

        // Length: acid tek uses mostly short notes but with
        // strategic holds for tension. Mental tribe = more holds.
        int len;
        float lr = rand01();
        if (lr < 0.55f) len = 1; // short 16th (most common)
        else if (lr < 0.80f) len = 2; // 8th note
        else if (lr < 0.93f) len = 3; // dotted 8th / hold
        else len = 4; // full beat hold (rare, tension)

        // Slide: happens when:
        //  a) this note is a longer hold (len >= 2) AND next step is active
        //  b) stepwise motion (small interval) — classic TB-303 portamento
        //  c) mental tribe: longer notes before octave jumps get slide
        bool slide = false;
        // We'll finalize slide after we know the next note — store for now
        motif[i] = { note, vel, len, false };

        // Retroactively assign slide to the PREVIOUS active step
        // if it was a long note and this step is nearby in pitch
        if (lastActiveIdx >= 0) {
            int interval = std::abs(note - lastNote);
            bool longPrev = (motif[lastActiveIdx].len >= 2);
            bool stepwise = (interval <= 5);
            bool bigJump = (interval >= 12); // octave jump — slide for mental feel
            // Slide on: stepwise motion, OR long note before jump (mental tribe)
            if (longPrev && (stepwise || bigJump) && rand01() < 0.55f)
                motif[lastActiveIdx].slide = true;
            // Short stepwise: occasional slide for standard acid
            else if (!longPrev && stepwise && rand01() < 0.25f)
                motif[lastActiveIdx].slide = true;
        }

        lastNote = note;
        lastActiveIdx = i;
    }

    // ── Phrase-end variation ──────────────────────────────
    // Tutorial: "change something at the end of the bar"
    // Last active step in bar resolves: to 5th (tension) or root (release)
    // and gets a slide INTO it from the step before
    if (lastActiveIdx >= 0 && lastActiveIdx > 8) {
        bool toFifth = (rand01() < 0.5f);
        motif[lastActiveIdx].note = ROOT + (toFifth ? 7 : 0);
        motif[lastActiveIdx].vel = 0.88f;
        motif[lastActiveIdx].slide = false; // clean landing

        // The step before the cadence gets a slide toward it
        for (int j = lastActiveIdx - 1; j >= 0; j--) {
            if (grid[j]) {
                motif[j].slide = true;
                // Approach: chromatic step below cadence note
                motif[j].note = motif[lastActiveIdx].note - 1;
                break;
            }
        }
    }

    // ── Optional: 32-step with variation ─────────────────
    // Second 16 steps = mutated copy (mental tribe complexity)
    bool do32 = (rand01() < 0.40f);

    // ── Write to sequence[] ───────────────────────────────
    for (int i = 0; i < SEQ_STEPS; i++) {
        int pos16 = i % 16;
        bool inVar = do32 && ((i % 32) >= 16);

        if (!grid[pos16]) continue;
        MotifStep m = motif[pos16];

        if (inVar) {
            // Variation: shift some notes up/down octave, flip a rest,
            // change a length — keeps the groove but adds movement
            float vr = rand01();
            if (vr < 0.18f) continue; // turn into rest
            else if (vr < 0.35f) m.note += 12; // octave up
            else if (vr < 0.45f) m.note -= 12; // octave down (if not too low)
            else if (vr < 0.55f) m.len = std::min(m.len + 1, 4); // hold longer
            // 45% unchanged — keeps motif recognisable
            m.note = std::max(24, std::min(m.note, 84)); // clamp to safe range
        }

        sequence[i].active = true;
        sequence[i].note = (uint8_t)m.note;
        sequence[i].velocity = m.vel;
        sequence[i].len = m.len + (m.slide ? 0.5 : 0);
        // sequence[i].slide = m.slide;
    }
}

}
