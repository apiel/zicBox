#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <vector>

#include "audio/engines/EngineBase.h"
#include "audio/sequencer/Generator.h"
#include "audio/sequencer/Step.h"

class SequenceBrain {
public:
    float bpm = 170.0f;
    bool isPlaying = false;

    // 64-Step Sequence
    std::vector<Step> kickSequence;

    static constexpr int NUM_PATTERNS = 8;
    float patternIdx = 0.0f;

    bool isNoteRepeatActive = false;
    int repeatDiv = 2; // Default 2 (1, 2, 4, 8 repeats per 16th step)
    double repeatSampleCounter = 0.0;

    uint8_t currentStep = 0;
    uint32_t midiTickCounter = 0; // 0..383 (384 ticks per 64-step loop)
    double sampleRate = 44100.0;
    double tickSampleCounter = 0.0;

    using MidiTxFunc = std::function<void(uint8_t)>;

    SequenceBrain(double sr = 44100.0)
        : sampleRate(sr)
    {
        kickSequence.resize(SEQ_STEPS);
        loadPattern(0);
    }

    void setSampleRate(double sr)
    {
        sampleRate = sr;
    }

    const char* getPatternName(int index) const
    {
        static const char* names[NUM_PATTERNS] = {
            "1. Basic 4/4",
            "2. Subtle Ghost",
            "3. Offbeat",
            "4. Driving",
            "5. Mental",
            "6. Rumble",
            "7. Peak",
            "8. Hard Techno"
        };
        if (index >= 0 && index < NUM_PATTERNS) {
            return names[index];
        }
        return "";
    }

    void loadPattern(int index)
    {
        index = std::clamp(index, 0, NUM_PATTERNS - 1);
        patternIdx = (float)index;

        if (index == 0) {
            // 1. Basic 4/4: Clean straight 4-on-the-floor
            for (int i = 0; i < SEQ_STEPS; i++) {
                kickSequence[i].active = (i % 4 == 0);
                kickSequence[i].note = 60;
                kickSequence[i].velocity = (i % 4 == 0) ? 1.0f : 0.0f;
            }
            return;
        }

        // Fixed seeds per pattern for deterministic, organic generation via Generator
        static const uint32_t seeds[NUM_PATTERNS] = {
            0,     // 1. Basic 4/4
            101,   // 2. Subtle Ghost
            202,   // 3. Offbeat Groove
            303,   // 4. Driving Techno
            404,   // 5. Mental Kick
            505,   // 6. Rumble Turnaround
            606,   // 7. Peak Bounce
            707    // 8. Hard Techno
        };

        // Generator parameters: { p1 = velocity, p2 = ghost density, p3 = end-loop rumble }
        static const float params[NUM_PATTERNS][3] = {
            { 0.50f, 0.00f, 0.00f }, // 1. Basic 4/4
            { 0.50f, 0.12f, 0.08f }, // 2. Subtle Ghost
            { 0.60f, 0.18f, 0.12f }, // 3. Offbeat Groove
            { 0.65f, 0.25f, 0.18f }, // 4. Driving Techno
            { 0.70f, 0.32f, 0.22f }, // 5. Mental Kick
            { 0.75f, 0.38f, 0.35f }, // 6. Rumble Turnaround
            { 0.80f, 0.45f, 0.30f }, // 7. Peak Bounce
            { 0.85f, 0.52f, 0.45f }  // 8. Hard Techno
        };

        Generator::gen.seed(seeds[index]);
        Generator::generateKick(kickSequence, params[index][0], params[index][1], params[index][2]);

        // Normalize all active steps to C3 (note 60)
        for (int i = 0; i < SEQ_STEPS; ++i) {
            if (kickSequence[i].active) {
                kickSequence[i].note = 60;
            }
        }

        // Add subtle, tasteful pitch variation (+1 or +2 semitones) on turnaround for select patterns
        switch (index) {
            case 2: // 3. Offbeat
                if (kickSequence[62].active) kickSequence[62].note = 61; // +1 semitone (C#3)
                break;
            case 3: // 4. Driving
                if (kickSequence[62].active) kickSequence[62].note = 61;
                break;
            case 4: // 5. Mental
                if (kickSequence[60].active) kickSequence[60].note = 61;
                if (kickSequence[62].active) kickSequence[62].note = 62; // +2 semitones (D3)
                break;
            case 5: // 6. Rumble
                if (kickSequence[62].active) kickSequence[62].note = 61;
                break;
            case 6: // 7. Peak
                if (kickSequence[60].active) kickSequence[60].note = 61;
                if (kickSequence[62].active) kickSequence[62].note = 62;
                break;
            case 7: // 8. Hard Techno
                if (kickSequence[62].active) kickSequence[62].note = 61;
                break;
            default:
                break;
        }
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
    bool processSample(IEngine& kick, const MidiTxFunc& txFunc = nullptr)
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
            currentStep = 0;
            uint8_t note0 = kickSequence[0].note ? kickSequence[0].note : 60;
            if (isNoteRepeatActive) {
                kick.noteOn(note0, kickSequence[0].velocity > 0.01f ? kickSequence[0].velocity : 1.0f);
                stepTriggered = true;
            } else if (kickSequence[0].active) {
                kick.noteOn(note0, kickSequence[0].velocity > 0.01f ? kickSequence[0].velocity : 1.0f);
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
            }

            uint8_t currentNote = kickSequence[currentStep].note ? kickSequence[currentStep].note : 60;

            if (isNoteRepeatActive) {
                int intervalTicks = 6 * std::clamp(repeatDiv, 1, 8);
                if ((midiTickCounter % intervalTicks) == 0) {
                    kick.noteOn(currentNote, kickSequence[currentStep].velocity > 0.01f ? kickSequence[currentStep].velocity : 1.0f);
                    stepTriggered = true;
                }
            } else if ((midiTickCounter % 6) == 0) {
                if (kickSequence[currentStep].active) {
                    kick.noteOn(currentNote, kickSequence[currentStep].velocity > 0.01f ? kickSequence[currentStep].velocity : 1.0f);
                    stepTriggered = true;
                }
            }

            midiTickCounter = (midiTickCounter + 1) % (SEQ_STEPS * 6);
        }

        return stepTriggered;
    }
};
