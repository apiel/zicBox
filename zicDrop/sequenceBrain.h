#pragma once

#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>

#include "zicXYv2/step.h"
#include "zicXYv2/generator.h"

class SequenceBrain {
public:
    // Clock parameters
    float bpm = 120.0f;
    float clockSpeedMultiplier = 1.0f; // For spacebar drop acceleration

    // Performance Drop
    bool spacebarHeld = false;

    // Running states
    double sampleRate = 44100.0;
    double sampleCounter = 0.0;
    uint32_t stepCounter = 0;

    // Trigger outputs for the current step
    bool triggerKick = false;
    bool triggerNoise = false;
    bool triggerAcid = false;
    float synthFollowMode = 0.0f; // 0: ALL, 1: DIV 2, 2: BEAT 1, 3: VEL 1.0
    uint32_t kickTriggerCounter = 0;
    float currentPitch = 0.0f; // Interpolated pitch value

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
    float kickP1 = 0.5f;
    float kickP2 = 0.5f;
    float kickP3 = 0.5f;

    SequenceBrain(double sr = 44100.0) : sampleRate(sr) {
        kickSequence.resize(SEQ_STEPS);
        regenerateKick();
    }

    void regenerateKick() {
        Generator::generateKick(kickSequence, kickP1, kickP2, kickP3);
    }

    void tick() {
        stepCounter++;

        // Shift Register visualization animation only (simulate life)
        bool lastBit = (shiftRegister & (1 << 15)) != 0;
        if (((float)rand() / (float)RAND_MAX) < 0.5f) {
            lastBit = !lastBit;
        }
        shiftRegister = (shiftRegister << 1) | (lastBit ? 1 : 0);

        // Derive gates from clock dividers
        gateDiv2  = (stepCounter % 2 == 0);
        gateDiv4  = (stepCounter % 4 == 0);
        gateDiv8  = (stepCounter % 8 == 0);
        gateDiv16 = (stepCounter % 16 == 0);
        gateDiv32 = (stepCounter % 32 == 0);

        // Index in the generated sequence
        uint32_t stepIdx = stepCounter % SEQ_STEPS;

        const Step& kickStep = kickSequence[stepIdx];

        triggerKick = kickStep.active;
        triggerAcid = false;

        if (triggerKick) {
            kickTriggerCounter++;
            int mode = (int)std::round(synthFollowMode);
            if (mode == 0) {
                triggerAcid = true;
            } else if (mode == 1) {
                if (kickTriggerCounter % 2 == 1) {
                    triggerAcid = true;
                }
            } else if (mode == 2) {
                if (stepIdx % 4 == 0) {
                    triggerAcid = true;
                }
            } else if (mode == 3) {
                if (kickStep.velocity >= 0.99f) {
                    triggerAcid = true;
                }
            }
        }
    }

    // Process every audio sample, returns true if a tick occurred
    bool processSample() {
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
