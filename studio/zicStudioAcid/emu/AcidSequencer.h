#pragma once

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <vector>

/**
 * AcidStep: Piano-roll grid step for Acid/Bass lines.
 *
 * Supports:
 * - Active flag (note on/off)
 * - Pitch (MIDI note number e.g. 24 = C1 up to 72 = C5)
 * - Length: note duration in steps/fractions (e.g. 1.0 = one 16th, 2.5 = 2.5 steps)
 * - Velocity: 0.0 .. 1.0 (drawn as bottom stem)
 * - Accent: accent trigger (boosts envelope & tone)
 * - Slide: legato glide towards next note
 */
struct AcidStep {
    bool on = false;
    uint8_t note = 36;       // MIDI Note (Default C2)
    float length = 1.0f;     // Duration in 16th steps (0.2 .. 8.0)
    float velocity = 0.8f;   // 0.0 .. 1.0
    bool accent = false;     // Accent trigger
    bool slide = false;      // Legato slide
};

class AcidSequencer {
public:
    static constexpr int STEPS = 32;       // 32-step sequence
    static constexpr int MIN_NOTE = 24;    // C1
    static constexpr int MAX_NOTE = 60;    // C4 (3 octaves)
    static constexpr int NUM_KEYS = MAX_NOTE - MIN_NOTE + 1; // 37 notes

    AcidStep steps[STEPS];
    int patternLength = 16;  // 16 or 32
    float bpm = 138.0f;
    int transpose = 0;       // Semitones transpose
    bool playing = false;
    std::atomic<int> currentStep { -1 };

    explicit AcidSequencer(float sampleRate)
        : sampleRate(sampleRate)
    {
        loadDefaultAcid();
    }

    void loadDefaultAcid()
    {
        for (int i = 0; i < STEPS; ++i) {
            steps[i] = AcidStep();
            steps[i].note = 36;
            steps[i].length = 0.8f;
        }

        // Classic driving Acid groove (16 steps)
        steps[0].on = true;  steps[0].note = 36; steps[0].accent = true;  steps[0].length = 0.9f; steps[0].velocity = 0.9f;
        steps[1].on = true;  steps[1].note = 36; steps[1].slide = false;  steps[1].length = 0.6f; steps[1].velocity = 0.7f;
        steps[2].on = true;  steps[2].note = 48; steps[2].slide = true;   steps[2].accent = true; steps[2].length = 1.2f; steps[2].velocity = 1.0f;
        // Step 3 rest
        steps[4].on = true;  steps[4].note = 36; steps[4].length = 0.7f;  steps[4].velocity = 0.8f;
        steps[5].on = true;  steps[5].note = 39; steps[5].slide = false;  steps[5].length = 0.5f; steps[5].velocity = 0.75f;
        steps[6].on = true;  steps[6].note = 41; steps[6].slide = true;   steps[6].length = 0.9f; steps[6].velocity = 0.85f;
        steps[7].on = true;  steps[7].note = 43; steps[7].slide = true;   steps[7].accent = true; steps[7].length = 1.0f; steps[7].velocity = 1.0f;

        steps[8].on = true;  steps[8].note = 36; steps[8].accent = true;  steps[8].length = 0.8f; steps[8].velocity = 0.9f;
        steps[9].on = true;  steps[9].note = 36; steps[9].length = 0.6f;  steps[9].velocity = 0.7f;
        steps[10].on = true; steps[10].note = 34; steps[10].length = 0.6f; steps[10].velocity = 0.75f;
        steps[11].on = true; steps[11].note = 36; steps[11].slide = true;  steps[11].length = 0.9f; steps[11].velocity = 0.85f;
        steps[12].on = true; steps[12].note = 36; steps[12].length = 0.7f; steps[12].velocity = 0.8f;
        // Step 13 rest
        steps[14].on = true; steps[14].note = 39; steps[14].accent = true; steps[14].slide = true; steps[14].length = 0.9f; steps[14].velocity = 0.95f;
        steps[15].on = true; steps[15].note = 38; steps[15].slide = true;  steps[15].length = 1.0f; steps[15].velocity = 0.9f;

        patternLength = 16;
    }

    void clear()
    {
        for (auto& s : steps) s = AcidStep();
    }

    void play()
    {
        stepIndex = -1;
        stepSamplesCounter = 0.0f;
        gateSamplesLeft = 0;
        playing = true;
    }

    void stop()
    {
        playing = false;
        currentStep = -1;
        gateSamplesLeft = 0;
    }

    void toggle() { playing ? stop() : play(); }

    struct TriggerEvent {
        bool noteOn = false;
        bool noteOff = false;
        uint8_t note = 0;
        float velocity = 1.0f;
        bool slide = false;
        bool accent = false;
    };

    TriggerEvent tick()
    {
        TriggerEvent ev;
        if (!playing) return ev;

        if (gateSamplesLeft > 0) {
            gateSamplesLeft--;
            if (gateSamplesLeft == 0) {
                ev.noteOff = true;
            }
        }

        if (stepSamplesCounter <= 0.0f) {
            float samplesPerStep = sampleRate * 60.0f / bpm * 0.25f; // 1/16th note
            stepSamplesCounter += samplesPerStep;

            stepIndex = (stepIndex + 1) % patternLength;
            currentStep = stepIndex;

            const AcidStep& s = steps[stepIndex];
            if (s.on) {
                ev.noteOn = true;
                ev.note = (uint8_t)std::clamp((int)s.note + transpose, 0, 127);
                ev.velocity = s.velocity;
                ev.slide = s.slide;
                ev.accent = s.accent;

                gateSamplesLeft = (int)(samplesPerStep * std::max(0.1f, s.length));
            }
        }

        stepSamplesCounter -= 1.0f;
        return ev;
    }

private:
    float sampleRate = 44100.0f;
    int stepIndex = -1;
    float stepSamplesCounter = 0.0f;
    int gateSamplesLeft = 0;
};
