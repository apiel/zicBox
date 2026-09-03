#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <vector>

#include "audio/engines/DriftKick.h"
#include "audio/sequencer/Generator.h"
#include "audio/sequencer/Step.h"

class SequenceBrain {
public:
    float bpm = 120.0f;
    bool isPlaying = false;

    // 64-Step Sequence
    std::vector<Step> kickSequence;

    // Generator parameters (0.0 to 1.0)
    float genP1 = 0.5f; // Velocity amount
    float genP2 = 0.2f; // Ghost note density
    float genP3 = 0.3f; // End rumble boost

    uint8_t currentStep = 0;
    uint32_t midiTickCounter = 0; // 0..383 (384 ticks per 64-step loop)
    double sampleRate = 44100.0;
    double tickSampleCounter = 0.0;

    using MidiTxFunc = std::function<void(uint8_t)>;

    SequenceBrain(double sr = 44100.0)
        : sampleRate(sr)
    {
        kickSequence.resize(SEQ_STEPS);
        regenerateKick();
    }

    void setSampleRate(double sr)
    {
        sampleRate = sr;
    }

    void regenerateKick()
    {
        Generator::generateKick(kickSequence, genP1, genP2, genP3);
    }

    void start(const MidiTxFunc& txFunc = nullptr)
    {
        isPlaying = true;
        midiTickCounter = 0;
        currentStep = 0;
        tickSampleCounter = 0.0;

        if (txFunc) {
            txFunc(0xFA); // MIDI Start
            txFunc(0xF8); // First MIDI Clock tick
        }
    }

    void stop(const MidiTxFunc& txFunc = nullptr)
    {
        isPlaying = false;
        if (txFunc) {
            txFunc(0xFC); // MIDI Stop
        }
    }

    void togglePlayStop(const MidiTxFunc& txFunc = nullptr)
    {
        if (isPlaying) {
            stop(txFunc);
        } else {
            start(txFunc);
        }
    }

    // Process single sample tick. Returns true if a step triggered
    bool processSample(DriftKick& kick, const MidiTxFunc& txFunc = nullptr)
    {
        if (!isPlaying) {
            return false;
        }

        // 24 PPQN = 24 ticks per quarter note
        // 1 Quarter note = 4 steps (16th notes)
        // 1 Step = 6 MIDI Clock ticks
        double samplesPerTick = (sampleRate * (60.0 / std::max(30.0f, bpm))) / 24.0;

        bool stepTriggered = false;

        // Check for step 0 trigger right on start
        if (tickSampleCounter == 0.0 && midiTickCounter == 0) {
            if (kickSequence[0].active) {
                kick.trigger(kickSequence[0].velocity);
                stepTriggered = true;
            }
            midiTickCounter = 1;
        }

        tickSampleCounter += 1.0;
        if (tickSampleCounter >= samplesPerTick) {
            tickSampleCounter -= samplesPerTick;

            if (txFunc) {
                txFunc(0xF8); // MIDI Clock byte
            }

            if ((midiTickCounter % 6) == 0) {
                currentStep = (midiTickCounter / 6) % SEQ_STEPS;
                if (kickSequence[currentStep].active) {
                    kick.trigger(kickSequence[currentStep].velocity);
                    stepTriggered = true;
                }
            }

            midiTickCounter = (midiTickCounter + 1) % (SEQ_STEPS * 6);
        }

        return stepTriggered;
    }
};
