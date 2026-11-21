#pragma once

/**
 * Emulator only: the final module has no sequencer, it just receives a trigger.
 * 64 steps of 1/16, each step is on/off + a semitone offset relative to the root note.
 */

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

struct Step {
    bool on = false;
    int8_t note = 0; // semitones relative to the Tune pot
};

class Sequencer {
public:
    static constexpr int STEPS = 64;

    Step steps[STEPS];
    float bpm = 180.0f;
    int transpose = 0; // semitones added to every step: the "tune" of the module
    bool playing = false;
    std::atomic<int> current { -1 }; // step being played, for the UI

    explicit Sequencer(float sampleRate)
        : sampleRate(sampleRate)
    {
        loadDefault();
    }

    void loadDefault()
    {
        for (auto& s : steps) s = Step();
        for (int i = 0; i < STEPS; i += 4) steps[i].on = true; // four on the floor
        steps[62].on = true; // small turnaround
        steps[63].on = true;
        steps[63].note = 2;
    }

    void clear()
    {
        for (auto& s : steps) s = Step();
    }

    void play()
    {
        index = -1;
        counter = 0.0f; // first tick fires step 0
        playing = true;
    }

    void stop()
    {
        playing = false;
        current = -1;
    }

    void toggle() { playing ? stop() : play(); }

    // Call once per audio sample. Returns true if a kick must be fired (note = semitone offset).
    bool tick(int& note)
    {
        if (!playing) return false;
        bool fire = false;
        if (counter <= 0.0f) {
            counter += sampleRate * 60.0f / bpm * 0.25f;
            index = (index + 1) % STEPS;
            current = index;
            if (steps[index].on) {
                note = steps[index].note;
                fire = true;
            }
        }
        counter -= 1.0f;
        return fire;
    }

private:
    const float sampleRate;
    int index = -1;
    float counter = 0.0f;
};
