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
    float synthTriggerStep = 0.0f; // 0: every, 1: /2, 2: /4, 3: /8, 4: /16, 5: /32
    float synthNoteCount = 4.0f;   // 1 to 12
    float synthArpStyle = 0.0f;    // 0 to 19 (20 different styles)
    int arpIndex = 0;
    int arpDirection = 1;
    int arpTick = 0;
    uint32_t kickTriggerCounter = 0;
    float currentPitch = 0.0f; // Scale offset value

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
        }

        // Synth sequencer trigger logic
        int divVal = (int)std::round(synthTriggerStep);
        int divisor = 1;
        if (divVal == 1) divisor = 2;
        else if (divVal == 2) divisor = 4;
        else if (divVal == 3) divisor = 8;
        else if (divVal == 4) divisor = 16;
        else if (divVal == 5) divisor = 32;

        if (stepCounter % divisor == 0) {
            triggerAcid = true;
            arpTick++;

            std::vector<int> scaleNotes = {0, 3, 5, 7, 10, 12, 15, 17, 19, 22, 24, 27};
            int N = std::max(1, std::min(12, (int)std::round(synthNoteCount) + 1));
            int arpStyle = (int)std::round(synthArpStyle);

            if (arpStyle == 0) { // UP
                arpIndex = arpTick % N;
            } else if (arpStyle == 1) { // DOWN
                arpIndex = (N - 1 - (arpTick % N));
            } else if (arpStyle == 2) { // UP-DOWN
                int period = (N > 1) ? (N - 1) * 2 : 1;
                int phase = arpTick % period;
                arpIndex = (phase < N) ? phase : period - phase;
            } else if (arpStyle == 3) { // DOWN-UP
                int period = (N > 1) ? (N - 1) * 2 : 1;
                int phase = arpTick % period;
                int upDown = (phase < N) ? phase : period - phase;
                arpIndex = N - 1 - upDown;
            } else if (arpStyle == 4) { // RANDOM
                arpIndex = std::rand() % N;
            } else if (arpStyle == 5) { // RANDOM NO REPEAT
                int nextIdx = std::rand() % N;
                if (N > 1 && nextIdx == arpIndex) nextIdx = (nextIdx + 1) % N;
                arpIndex = nextIdx;
            } else if (arpStyle == 6) { // DRUNK
                int r = std::rand() % 3 - 1;
                arpIndex = std::max(0, std::min(N - 1, arpIndex + r));
            } else if (arpStyle == 7) { // CONVERGE
                int step = arpTick % N;
                if (step % 2 == 0) arpIndex = step / 2;
                else arpIndex = N - 1 - (step / 2);
            } else if (arpStyle == 8) { // DIVERGE
                int step = arpTick % N;
                int mid = N / 2;
                if (step % 2 == 0) arpIndex = mid + (step / 2);
                else arpIndex = mid - 1 - (step / 2);
            } else if (arpStyle == 9) { // UP 2 DOWN 1
                int step = arpTick % N;
                int base = (step / 2) * 2;
                if (step % 2 == 0) arpIndex = base;
                else arpIndex = base + 2;
                arpIndex = arpIndex % N;
            } else if (arpStyle == 10) { // DOWN 2 UP 1
                int step = arpTick % N;
                int base = (step / 2) * 2;
                if (step % 2 == 0) arpIndex = N - 1 - base;
                else arpIndex = N - 1 - (base + 2);
                if (arpIndex < 0) arpIndex += N;
                arpIndex = arpIndex % N;
            } else if (arpStyle == 11) { // TRILL 0
                if (arpTick % 2 == 0) {
                    arpIndex = 0;
                } else {
                    arpIndex = (1 + (arpTick / 2)) % N;
                    if (arpIndex == 0 && N > 1) arpIndex = 1;
                }
            } else if (arpStyle == 12) { // TRILL MAX
                if (arpTick % 2 == 0) {
                    arpIndex = N - 1;
                } else {
                    arpIndex = (N > 1) ? (arpTick / 2) % (N - 1) : 0;
                }
            } else if (arpStyle == 13) { // STEP DUP
                arpIndex = (arpTick / 2) % N;
            } else if (arpStyle == 14) { // SKIP 1
                int half = (N + 1) / 2;
                int step = arpTick % N;
                if (step < half) {
                    arpIndex = step * 2;
                } else {
                    arpIndex = 1 + (step - half) * 2;
                }
                arpIndex = arpIndex % N;
            } else if (arpStyle == 15) { // SKIP 2
                int step = arpTick % N;
                arpIndex = (step * 3) % N;
            } else if (arpStyle == 16) { // TURING 3
                arpIndex = (arpTick * 3) % N;
            } else if (arpStyle == 17) { // TURING 5
                arpIndex = (arpTick * 5) % N;
            } else if (arpStyle == 18) { // OCTAVE JUMP
                int half = N / 2;
                if (half == 0) half = 1;
                if (arpTick % 2 == 0) {
                    arpIndex = (arpTick / 2) % half;
                } else {
                    arpIndex = half + ((arpTick / 2) % (N - half));
                }
                arpIndex = arpIndex % N;
            } else if (arpStyle == 19) { // STACCATO STEP
                int sub = arpTick % 3;
                int noteIdx = (arpTick / 3) % N;
                if (sub == 2) arpIndex = N - 1;
                else arpIndex = noteIdx;
            }

            arpIndex = std::max(0, std::min(N - 1, arpIndex));
            currentPitch = (float)scaleNotes[arpIndex];
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
