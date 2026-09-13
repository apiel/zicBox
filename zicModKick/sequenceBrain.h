#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <vector>

#include "audio/engines/EngineBase.h"
#include "audio/sequencer/Generator.h"
#include "audio/sequencer/Step.h"

struct Pattern {
    const char* name;
    uint8_t trig[64];
};

class SequenceBrain {
public:
    float bpm = 170.0f;
    bool isPlaying = false;

    // 64-Step Sequence
    std::vector<Step> kickSequence;

    static constexpr int NUM_PATTERNS = 17;
    float patternIdx = 0.0f;

    inline static const Pattern patterns[NUM_PATTERNS] = {
        { "1. Basic 4/4",         { 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0 } },
        { "2. Offbeat Ghost 1",   { 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,61,0 } },
        { "3. Offbeat Ghost 2",   { 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,61,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,59,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,61,0 } },
        { "4. Driving Syncop",    { 60,0,0,0, 60,0,0,60, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,59,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,60, 60,0,0,0, 60,0,61,0 } },
        { "5. Turnaround Pitch 1",{ 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,62,0, 60,0,0,0, 60,0,61,0, 60,0,0,0, 61,0,62,0 } },
        { "6. Turnaround Pitch 2",{ 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 61,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 61,0,62,0 } },
        { "7. Tribal Bounce",     { 60,0,0,60, 60,0,0,0, 60,0,0,60, 60,0,60,0, 60,0,0,60, 60,0,0,0, 60,0,0,60, 60,0,60,0, 60,0,0,60, 60,0,0,0, 60,0,0,60, 60,0,60,0, 60,0,0,60, 60,0,0,0, 60,0,0,60, 60,0,61,0 } },
        { "8. Double Hit",        { 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 61,0,61,0 } },
        { "9. Industrial Offbeat",{ 60,0,0,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,60, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 61,0,62,0 } },
        { "10. Pitch Glide 1",    { 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,61,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,61,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 61,0,62,0 } },
        { "11. Pitch Glide 2",    { 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,60,0, 61,0,62,0 } },
        { "12. Hard Groove 1",    { 60,0,0,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,0,0, 60,0,60,0, 60,0,61,0, 60,0,0,0, 60,0,0,0, 61,0,0,0, 61,0,62,0 } },
        { "13. Hard Groove 2",    { 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,61,0, 60,0,0,0, 60,0,61,0, 61,0,0,0, 61,0,62,0 } },
        { "14. Hard Turnaround", { 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,61,0, 60,0,0,0, 60,0,0,0, 61,0,61,0, 61,0,62,0 } },
        { "15. Hard Peak",       { 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,61,0, 60,0,0,0, 60,0,0,0, 61,0,61,0, 61,0,62,0 } },
        { "16. Hard Techno",      { 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,61,0, 60,0,0,0, 60,0,0,0, 61,0,0,0, 61,0,62,0 } },
        { "17. Tek", { 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,60,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 60,0,0,0, 61,0,0,0, 62,0,62,0 } },
    };

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
        generateStyle16Pattern();
    }

    void setSampleRate(double sr)
    {
        sampleRate = sr;
    }

    const char* getPatternName(int index) const
    {
        if (index >= 0 && index < NUM_PATTERNS) {
            return patterns[index].name;
        }
        return "";
    }

    void loadPattern(int index)
    {
        index = std::clamp(index, 0, NUM_PATTERNS - 1);
        patternIdx = (float)index;
        const Pattern& p = patterns[index];
        for (int i = 0; i < SEQ_STEPS; ++i) {
            uint8_t noteVal = p.trig[i];
            if (noteVal > 0) {
                kickSequence[i].active = true;
                kickSequence[i].note = noteVal;
                kickSequence[i].velocity = (i % 4 == 0) ? 1.0f : 0.65f;
            } else {
                kickSequence[i].active = false;
                kickSequence[i].note = 60;
                kickSequence[i].velocity = 0.8f;
            }
        }
    }

    void generateStyle16Pattern()
    {
        // 1. Clear & set base 4-on-the-floor
        for (int i = 0; i < SEQ_STEPS; ++i) {
            kickSequence[i].active = (i % 4 == 0);
            kickSequence[i].note = 60;
            kickSequence[i].velocity = (i % 4 == 0) ? 1.0f : 0.65f;
        }

        // 2. Bar 1 (0..15): optional ghost note on step 10 or 14
        if (Generator::rand01() < 0.70f) {
            int gStep = (Generator::rand01() < 0.5f) ? 14 : 10;
            kickSequence[gStep].active = true;
            kickSequence[gStep].note = 60;
            kickSequence[gStep].velocity = 0.60f;
        }

        // 3. Bar 2 (16..31): optional ghost note on step 22 or 30
        if (Generator::rand01() < 0.70f) {
            int gStep = (Generator::rand01() < 0.5f) ? 30 : 22;
            kickSequence[gStep].active = true;
            kickSequence[gStep].note = 60;
            kickSequence[gStep].velocity = 0.60f;
        }

        // 4. Bar 3 (32..47): pitched ghost note (+1 semitone, C#3 / note 61)
        if (Generator::rand01() < 0.85f) {
            int gStep = (Generator::rand01() < 0.5f) ? 46 : 42;
            kickSequence[gStep].active = true;
            kickSequence[gStep].note = 61;
            kickSequence[gStep].velocity = 0.65f;
        }

        // 5. Bar 4 turnaround (48..63): pitched driving turnaround fill
        int turnStep1 = (Generator::rand01() < 0.5f) ? 56 : 54;
        kickSequence[turnStep1].active = true;
        kickSequence[turnStep1].note = 61;
        kickSequence[turnStep1].velocity = 0.85f;

        kickSequence[60].active = true;
        kickSequence[60].note = (Generator::rand01() < 0.5f) ? 61 : 62;
        kickSequence[60].velocity = 0.90f;

        kickSequence[62].active = true;
        kickSequence[62].note = (Generator::rand01() < 0.5f) ? 62 : 63;
        kickSequence[62].velocity = 1.0f;

#ifdef DRAW_DESKTOP
        // Print generated pattern array to console for easy copy-pasting
        printf("\nGenerated Pattern:\n");
        printf("{ \"Generated\", { ");
        for (int i = 0; i < SEQ_STEPS; ++i) {
            uint8_t note = kickSequence[i].active ? kickSequence[i].note : 0;
            printf("%d", note);
            if (i < SEQ_STEPS - 1) {
                if (i % 4 == 3) {
                    printf(", ");
                } else {
                    printf(",");
                }
            }
        }
        printf(" } }\n\n");
#endif
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
