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

void generatePattern(std::vector<Step>& sequence, float rumbleParam, float ghostParam)
{
    for (int i = 0; i < SEQ_STEPS; i++) {
        sequence[i].active = false;
        sequence[i].note = 60;
        sequence[i].velocity = 1.0f;
        sequence[i].condition = 1.0f;
        sequence[i].len = 1.0f;
    }

    // p2: Ghost note density
    float baseGhostChance = ghostParam * 0.36f; // range 0% .. 36% (default 0.18f)

    // p3: End-of-loop rumble probability boost
    float endRumbleBoost = rumbleParam * 0.4f;

    bool lastHasGhost = false;
    for (int i = 0; i < SEQ_STEPS; i += 4) {
        // Main Tribe Kick
        sequence[i].active = true;
        sequence[i].velocity = 1.0f;
        sequence[i].note = 60;
        sequence[i].condition = 1.0f;

        lastHasGhost = false;
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
                    lastHasGhost = true;
                }
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

    // Generator parameters: 2 independent params for Rumble and Ghost
    float kickRumbleParam = 0.5f; // 0.0 to 1.0: End-of-phrase rumble roll density
    float kickGhostParam = 0.5f; // 0.0 to 1.0: Syncopated ghost kick density

    SequenceBrain(double sr = 44100.0)
        : sampleRate(sr)
    {
        kickSequence.resize(SEQ_STEPS);
        regenerateKick();
    }

    void regenerateKick()
    {
        Generator::generatePattern(kickSequence, kickRumbleParam, kickGhostParam);
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
