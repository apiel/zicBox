/** Description:
This code defines the blueprint for a digital music **Sequencer**, a component essential for scheduling and triggering timed events, typically musical notes. It organizes a series of steps that dictate when and how sounds should be played.

The core of the system is the **Step** structure, which holds all parameters for a single event:
*   Its specific position in the sequence timeline.
*   The musical note value (pitch).
*   The velocity (volume or intensity).
*   The duration (how long the note sustains).
*   A "condition" value, which is a probability setting determining the likelihood of the step actually playing when its position is reached.

The `Sequencer` tracks the current time using a running step counter. Every time the sequence advances (using the `onStep` function), it checks if any stored step matches the current time position.

If a step is due:
1.  It may use a dedicated random noise generator (from the referenced `LookupTable`) to check the "condition." If the step fails the probability check, it is skipped.
2.  If the step passes, it triggers a "Note On" signal and starts a countdown based on its duration.
3.  When that duration countdown reaches zero, a "Note Off" signal is triggered.

The sequencer is designed to be highly controlled, employing helper functions to ensure values like velocity and condition are clamped (kept within safe, predefined minimum and maximum limits). It also includes methods to efficiently save (`serialize`) and load (`hydrate`) the entire musical pattern for storage or recall.

sha: 51f1b3168c3ddc927edbae97da1bc3813f73cd504879ef111687ae67d9fc8fcb
*/
#pragma once

#include "audio/utils/noise.h"
#include "helpers/clamp.h"

#include <array>
#include <cstdint>
#include <utility>
#include <cstddef>

class Sequencer {
public:
    static constexpr size_t MAX_STEPS = 128;

    struct RawStep {
        uint16_t pos;
        uint16_t len;
        uint8_t note;
        uint8_t vel;  // 0-100
        uint8_t cond; // 0-100
    };

    struct Step {
        uint16_t position = 0;
        uint8_t note = 60;
        float velocity = 0.8f;
        uint16_t len = 1;
        float condition = 1.0f;
        uint16_t activeCounter = 0;
        bool isPlaying = false;
    };

    enum class EventType { None, NoteOn, NoteOff };
    struct Event {
        EventType type = EventType::None;
        const Step* stepRef = nullptr;
    };

private:
    std::array<Step, MAX_STEPS> steps;
    size_t stepUsage = 0;
    uint32_t stepCounter = 0;
    uint16_t sequenceLength = 64;

public:
    void setStepCount(uint16_t value) { sequenceLength = CLAMP(value, 4, 512); }
    uint16_t getStepCount() const { return sequenceLength; }
    void reset() { stepCounter = 0; }

    Step* getStep(size_t index) { 
        return (index < MAX_STEPS) ? &steps[index] : nullptr; 
    }

    void setCondition(Step& step, float condition) {
        step.condition = CLAMP(condition, 0.0f, 1.0f);
    }

    void setNote(Step& step, int note) {
        step.note = (uint8_t)CLAMP(note, 0, 127);
    }

    void setVelocity(Step& step, float velocity) {
        step.velocity = CLAMP(velocity, 0.0f, 1.0f);
    }

    void setPosition(Step& step, uint16_t pos) {
        step.position = CLAMP(pos, 0, sequenceLength - 1);
    }

    // --- Serialization / Hydration ---
    void setStepUsage(size_t count) { stepUsage = CLAMP(count, 0, MAX_STEPS); }
    size_t getStepUsage() const { return stepUsage; }

    void hydrate(const RawStep* data, size_t count) {
        stepUsage = (count > MAX_STEPS) ? MAX_STEPS : count;
        for (size_t i = 0; i < stepUsage; ++i) {
            steps[i].position = data[i].pos;
            steps[i].note     = data[i].note;
            steps[i].velocity = data[i].vel / 100.0f;
            steps[i].len      = data[i].len;
            steps[i].condition = data[i].cond / 100.0f;
            steps[i].activeCounter = 0;
            steps[i].isPlaying     = false;
        }
    }

    auto onStep() {
        std::array<Event, 8> eventQueue;
        size_t eventIdx = 0;
        processLogic([&](const Event& ev) {
            if (eventIdx < eventQueue.size()) eventQueue[eventIdx++] = ev;
        });
        return std::make_pair(eventQueue, eventIdx);
    }

    Event onStepMono() {
        Event lastEvent = { EventType::None, nullptr };
        processLogic([&](const Event& ev) { lastEvent = ev; });
        return lastEvent;
    }

private:
    template <typename F>
    void processLogic(F&& callback) {
        stepCounter++;
        if (stepCounter >= sequenceLength) stepCounter = 0;

        for (size_t i = 0; i < stepUsage; ++i) {
            auto& s = steps[i];

            // Note Off
            if (s.isPlaying && s.activeCounter > 0) {
                if (--s.activeCounter == 0) {
                    s.isPlaying = false;
                    callback({ EventType::NoteOff, &s });
                }
            }

            // Note On
            if (stepCounter == s.position && s.velocity > 0.0f) {
                if (s.condition >= 1.0f || (Noise::get() <= s.condition)) {
                    s.activeCounter = s.len;
                    s.isPlaying = true;
                    callback({ EventType::NoteOn, &s });
                }
            }
        }
    }
};

// // Inside your Audio Engine class
// void processTick() {
//     auto [events, count] = sequencer.onStep();
    
//     for (size_t i = 0; i < count; ++i) {
//         if (events[i].type == EventType::NoteOn) {
//             this->triggerSynth(events[i].stepRef->note); // 'this' works fine!
//         } else {
//             this->releaseSynth(events[i].stepRef->note);
//         }
//     }
// }