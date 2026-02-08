#pragma once

#include "audio/utils/noise.h"
#include "helpers/clamp.h"

#include <array>
#include <cstddef>
#include <cstdint>

#ifndef MAX_NOTES_PER_STEP
#define MAX_NOTES_PER_STEP 4
#endif

class Sequencer {
public:
    static constexpr size_t MAX_STEPS = 128;
    static constexpr int8_t EMPTY_NOTE = -1;

    struct Step {
        int8_t notes[MAX_NOTES_PER_STEP];
        float velocity = 0.8f;
        uint16_t len = 1;
        float condition = 1.0f;
        bool enabled = false;

        void clear()
        {
            enabled = false;
            for (auto& n : notes)
                n = EMPTY_NOTE;
            notes[0] = 60; // set first note to C4 by default
        }
    };

    enum class EventType { NoteOn,
        NoteOff };
    struct Event {
        EventType type;
        const Step* stepRef;
    };

private:
    std::array<Step, MAX_STEPS> steps;
    uint16_t sequenceLength = 16;
    uint32_t stepCounter = 0;

    struct ActiveStep {
        uint16_t remainingTicks = 0;
        uint16_t stepIndex = 0;
        bool active = false;
    };
    // Keep track of up to 8 active steps
    std::array<ActiveStep, 8> activeSlots;

public:
    Sequencer()
    {
        for (auto& s : steps)
            s.clear();
        for (auto& a : activeSlots)
            a.active = false;
    }

    Step& getStep(size_t index) { return steps[index % MAX_STEPS]; }
    void setStepCount(uint16_t value) { sequenceLength = CLAMP(value, 1, MAX_STEPS); }
    uint16_t getStepCount() const { return sequenceLength; }

    template <typename F>
    void onStep(F&& callback)
    {
        // Note Off
        for (auto& slot : activeSlots) {
            if (slot.active && --slot.remainingTicks == 0) {
                slot.active = false;
                callback({ EventType::NoteOff, &steps[slot.stepIndex] });
            }
        }

        stepCounter = (stepCounter + 1) % sequenceLength;
        Step& s = steps[stepCounter];

        // Note On
        if (s.enabled && s.velocity > 0.0f) {
            if (s.condition >= 1.0f || (Noise::get() <= s.condition)) {
                callback({ EventType::NoteOn, &s });

                // Register for Note Off tracking
                for (auto& slot : activeSlots) {
                    if (!slot.active) {
                        slot.active = true;
                        slot.stepIndex = stepCounter;
                        slot.remainingTicks = s.len;
                        break;
                    }
                }
            }
        }
    }
};

// sequencer.onStep([&](const Sequencer::Event& ev) {
//     if (ev.type == Sequencer::EventType::NoteOn) {
//         for (int8_t note : ev.stepRef->notes) {
//             if (note != Sequencer::EMPTY_NOTE) {
//                 synth.trigger(note, ev.stepRef->velocity);
//             }
//         }
//     } else {
//         for (int8_t note : ev.stepRef->notes) {
//             if (note != Sequencer::EMPTY_NOTE) synth.release(note);
//         }
//     }
// });