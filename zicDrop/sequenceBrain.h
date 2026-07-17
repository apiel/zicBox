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
    std::vector<Step> synthSequence;

    // Generator parameters
    float kickP1 = 0.5f;
    float kickP2 = 0.5f;
    float kickP3 = 0.5f;

    float synthP1 = 0.5f;
    float synthP2 = 0.5f;
    float synthP3 = 0.5f;
    float synthStretch = 4.0f; // Stretch parameter: index mapped to 4, 8, 16, 32, 64, 128

    SequenceBrain(double sr = 44100.0) : sampleRate(sr) {
        kickSequence.resize(SEQ_STEPS);
        synthSequence.resize(SEQ_STEPS);
        regenerateKick();
        regenerateSynth();
    }

    int getStretchVal() const {
        int idx = (int)std::round(synthStretch);
        if (idx == 0) return 4;
        if (idx == 1) return 8;
        if (idx == 2) return 16;
        if (idx == 3) return 32;
        if (idx == 4) return 64;
        return 128;
    }

    void stretchSequence(std::vector<Step>& seq) {
        std::vector<Step> newSeq(SEQ_STEPS);
        for (int i = 0; i < 32; i++) {
            if (seq[i].active) {
                newSeq[i * 2] = seq[i];
                newSeq[i * 2].len *= 2.0f;
            }
        }
        seq = newSeq;
    }

    void compressSequence(std::vector<Step>& seq) {
        std::vector<Step> newSeq(SEQ_STEPS);
        for (int i = 0; i < SEQ_STEPS; i++) {
            if (seq[i].active) {
                int newIdx = i / 2;
                newSeq[newIdx] = seq[i];
                newSeq[newIdx].active = true;
                newSeq[newIdx].len = std::max(0.5f, newSeq[newIdx].len / 2.0f);
            }
        }
        for (int i = 0; i < SEQ_STEPS; i++) {
            if (i > 31) seq[i] = newSeq[i - 32];
            else seq[i] = newSeq[i];
        }
    }

    void regenerateKick() {
        Generator::generateKick(kickSequence, kickP1, kickP2, kickP3);
    }

    void regenerateSynth() {
        Generator::generateBass(synthSequence, synthP1, synthP2, synthP3);
        int stretchVal = getStretchVal();
        if (stretchVal == 32) {
            stretchSequence(synthSequence);
        } else if (stretchVal == 16) {
            stretchSequence(synthSequence);
            stretchSequence(synthSequence);
        } else if (stretchVal == 8) {
            stretchSequence(synthSequence);
            stretchSequence(synthSequence);
            stretchSequence(synthSequence);
        } else if (stretchVal == 4) {
            stretchSequence(synthSequence);
            stretchSequence(synthSequence);
            stretchSequence(synthSequence);
            stretchSequence(synthSequence);
        } else if (stretchVal == 128) {
            compressSequence(synthSequence);
        }
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
        const Step& synthStep = synthSequence[stepIdx];

        triggerKick = kickStep.active;
        triggerAcid = synthStep.active;

        if (triggerAcid) {
            currentPitch = (float)synthStep.note;
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
