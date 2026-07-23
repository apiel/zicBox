#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <random>
#include <vector>

#include "zicXYv2/step.h"

namespace Generator {
static std::random_device rd;
static std::mt19937 gen(rd());

float rand01()
{
    return std::uniform_real_distribution<float>(0.0f, 1.0f)(gen);
}

void generateKick(std::vector<Step>& sequence, float param)
{
    for (int i = 0; i < SEQ_STEPS; i++) {
        sequence[i].active = false;
        sequence[i].note = 60;
        sequence[i].velocity = 1.0f;
        sequence[i].condition = 1.0f;
        sequence[i].len = 1.0f;
    }

    // 1. Base 4-on-the-floor kick (steps 0, 4, 8, 12...)
    for (int i = 0; i < SEQ_STEPS; i += 4) {
        sequence[i].active = true;
    }

    // At 0%, strictly 4-on-the-floor
    if (param <= 0.001f) return;

    float p = std::clamp(param, 0.0f, 1.0f);

    for (int bar = 0; bar < SEQ_STEPS; bar += 16) {
        // --- A. Offbeat Bounce (Steps 6, 10) ---
        if (rand01() < (p * 0.70f)) sequence[bar + 6].active = true;  // Beat 2 offbeat
        if (rand01() < (p * 0.55f)) sequence[bar + 10].active = true; // Beat 3 offbeat

        // --- B. Phrase-End Rumble Rolls (Beat 4: Steps 14, 15, 13) ---
        if (rand01() < (p * 0.85f)) {
            sequence[bar + 14].active = true; // Beat 4 offbeat
        }
        if (sequence[bar + 14].active && rand01() < (p * 0.65f)) {
            sequence[bar + 15].active = true; // Roll into next bar
        }
        if (p > 0.6f && rand01() < ((p - 0.4f) * 0.50f)) {
            sequence[bar + 13].active = true; // Triplet entry into beat 4 roll
        }

        // --- C. Controlled Syncopated Ghosts (> 50% on knob) ---
        if (p > 0.5f) {
            float syncopStrength = (p - 0.5f) * 2.0f; // 0.0 to 1.0

            if (rand01() < (syncopStrength * 0.40f)) {
                sequence[bar + 2].active = true; // Beat 1 offbeat
            }

            if (sequence[bar + 6].active && rand01() < (syncopStrength * 0.35f)) {
                sequence[bar + 7].active = true; // Syncopated 16th
            }
        }
    }
}
} // namespace Generator
class SequenceBrain {
public:
    // Clock parameters
    float bpm = 180.0f;
    float clockSpeedMultiplier = 1.0f; // For spacebar drop acceleration

    // Performance Drop
    bool spacebarHeld = false;
    bool performanceMode = false;
    bool lastSpacebarHeld = false;

    // Running states
    double sampleRate = 44100.0;
    double sampleCounter = 0.0;
    uint32_t stepCounter = 0;

    bool triggerKick = false;
    bool triggerNoise = false;
    bool triggerSynth = false;
    float synthTriggerStep = 0.0f; // 0: follow, 1: 1, 2: 2, etc.
    uint32_t kickTriggerCounter = 0;
    float currentPitch = 0.0f; // Scale offset value (now always 0)

    struct TrigOption {
        const char* label;
        int divisor;
        int offset;
    };
    inline static const TrigOption trigOptions[16] = {
        { "follow", 0, 0 },
        { "1", 1, 0 },
        { "2", 2, 0 },
        { "2-off", 2, 1 },
        { "4", 4, 0 },
        { "4-off", 4, 2 },
        { "4-rumble", 4, 3 },
        { "8", 8, 0 },
        { "8-off", 8, 4 },
        { "8-rumble", 8, 7 },
        { "16", 16, 0 },
        { "16-off", 16, 8 },
        { "16-rumble", 16, 15 },
        { "32", 32, 0 },
        { "32-off", 32, 16 },
        { "32-rumble", 32, 31 }
    };

    // Clock Dividers states
    bool gateDiv2 = false;
    bool gateDiv4 = false;
    bool gateDiv8 = false;

    bool gateDiv16 = false;
    bool gateDiv32 = false;

    // Shift Register for UI Visualizer only (Turing machine is removed)
    uint16_t shiftRegister = 0xACE1;

    // Sequences
    std::vector<Step> kickSequence;

    // Generator parameters
    float kickGenParam = 0.0f;
    float kickRumbleParam = 0.0f;
    float kickGhostParam = 0.0f;

    SequenceBrain(double sr = 44100.0)
        : sampleRate(sr)
    {
        kickSequence.resize(SEQ_STEPS);
        regenerateKick();
    }

    void regenerateKick()
    {
        Generator::generateKick(kickSequence, kickGenParam);
    }

    void tick()
    {
        stepCounter++;

        // Shift Register visualization animation only (simulate life)
        bool lastBit = (shiftRegister & (1 << 15)) != 0;
        if (((float)rand() / (float)RAND_MAX) < 0.5f) {
            lastBit = !lastBit;
        }
        shiftRegister = (shiftRegister << 1) | (lastBit ? 1 : 0);

        // Derive gates from clock dividers
        gateDiv2 = (stepCounter % 2 == 0);
        gateDiv4 = (stepCounter % 4 == 0);
        gateDiv8 = (stepCounter % 8 == 0);
        gateDiv16 = (stepCounter % 16 == 0);
        gateDiv32 = (stepCounter % 32 == 0);

        // Index in the generated sequence
        uint32_t stepIdx = stepCounter % SEQ_STEPS;

        const Step& kickStep = kickSequence[stepIdx];

        triggerKick = kickStep.active;
        triggerSynth = false;

        if (triggerKick) {
            kickTriggerCounter++;
        }

        // Synth sequencer trigger logic
        int divVal = (int)std::round(synthTriggerStep);
        divVal = std::clamp(divVal, 0, 15);
        int divisor = trigOptions[divVal].divisor;
        int offset = trigOptions[divVal].offset;

        if (divisor == 0) {
            if (triggerKick) {
                triggerSynth = true;
                currentPitch = 0.0f;
            }
        } else if ((stepCounter % divisor) == offset) {
            triggerSynth = true;
            currentPitch = 0.0f;
        }

        performanceMode = spacebarHeld;
        lastSpacebarHeld = spacebarHeld;
    }

    // Process every audio sample, returns true if a tick occurred
    bool processSample()
    {
        // Calculate dynamic BPM
        float effectiveBpm = bpm;

        // Samples per step (assuming 16th notes / 4 ticks per beat)
        double samplesPerStep = (60.0 / effectiveBpm) * sampleRate / 4.0;

        sampleCounter += 1.0;
        if (sampleCounter >= samplesPerStep) {
            sampleCounter = 0.0;
            tick();
            return true;
        }
        return false;
    }
};
