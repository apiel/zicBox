#pragma once

#include <algorithm>
#include <atomic>
#include <cstdint>

/**
 * Fm4Step: one 16th-note step of the FM4 step sequencer.
 *
 * - on       note gate on/off
 * - note     MIDI note number (36 = C2)
 * - length   gate length in 16th steps (0.2 .. 8.0)
 * - velocity 0 .. 1
 * - accent   accent trigger (brighter, louder)
 * - slide    legato glide into the next note
 */
struct Fm4Step {
    bool on = false;
    uint8_t note = 36;
    float length = 0.85f;
    float velocity = 0.8f;
    bool accent = false;
    bool slide = false;
};

class Fm4Sequencer {
public:
    static constexpr int STEPS = 32;        // 32-step sequence
    static constexpr int MIN_NOTE = 24;     // C1
    static constexpr int MAX_NOTE = 84;     // C6

    Fm4Step steps[STEPS];
    int patternLength = 16;                 // 16 or 32
    float bpm = 120.0f;
    int transpose = 0;                      // semitones
    bool playing = false;
    std::atomic<int> currentStep { -1 };

    explicit Fm4Sequencer(float sampleRate)
        : sampleRate(sampleRate)
    {
        loadDefault();
    }

    void loadDefault()
    {
        for (int i = 0; i < STEPS; ++i) steps[i] = Fm4Step();

        // A dark FM groove in C minor: C2 / Eb2 / F2 / G2 / Bb2 / C3
        auto set = [&](int i, int note, float len, float vel, bool accent = false, bool slide = false) {
            steps[i].on = true;
            steps[i].note = (uint8_t)note;
            steps[i].length = len;
            steps[i].velocity = vel;
            steps[i].accent = accent;
            steps[i].slide = slide;
        };

        set(0, 36, 0.90f, 0.95f, true);
        set(1, 36, 0.60f, 0.70f);
        set(2, 48, 1.20f, 1.00f, true, true);
        // 3: rest
        set(4, 36, 0.70f, 0.80f);
        set(5, 39, 0.50f, 0.75f);
        set(6, 41, 0.90f, 0.85f, false, true);
        set(7, 43, 1.00f, 1.00f, true, true);
        set(8, 36, 0.80f, 0.90f, true);
        set(9, 36, 0.60f, 0.70f);
        set(10, 34, 0.60f, 0.75f);
        set(11, 36, 0.90f, 0.85f, false, true);
        set(12, 36, 0.70f, 0.80f);
        // 13: rest
        set(14, 39, 0.90f, 0.95f, true, true);
        set(15, 38, 1.00f, 0.90f, false, true);

        patternLength = 16;
    }

    void clear()
    {
        for (auto& s : steps) s = Fm4Step();
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
            if (gateSamplesLeft == 0) ev.noteOff = true;
        }

        if (stepSamplesCounter <= 0.0f) {
            float samplesPerStep = sampleRate * 60.0f / bpm * 0.25f; // 1/16th
            stepSamplesCounter += samplesPerStep;

            stepIndex = (stepIndex + 1) % std::clamp(patternLength, 1, STEPS);
            currentStep = stepIndex;

            const Fm4Step& s = steps[stepIndex];
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
