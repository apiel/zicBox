#pragma once

#include "audio/utils/noise.h"
#include "helpers/clamp.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

class Sequencer {
public:
    static constexpr size_t MAX_STEPS = 128;

    struct Step {
        uint16_t position = 0;
        uint8_t note = 60;
        float velocity = 0.8f;
        uint16_t len = 1;
        float condition = 1.0f;
        uint16_t activeCounter = 0;
    };

    std::array<Step, MAX_STEPS> steps;
    size_t usedStepCount = 0;

    enum class EventType { None,
        NoteOn,
        NoteOff };
    struct Event {
        EventType type = EventType::None;
        const Step* stepRef = nullptr;
    };

private:
    uint32_t stepCounter = 0;
    uint16_t sequenceLength = 64;

public:
    void setStepCount(uint16_t value) { sequenceLength = CLAMP(value, 4, 512); }
    uint16_t getStepCount() const { return sequenceLength; }
    void reset() { stepCounter = 0; }

    size_t getExistingStepCount() const { return usedStepCount; }
    Step* getStep(size_t index)
    {
        return (index < usedStepCount) ? &steps[index] : nullptr;
    }

    void addStep(Step& step)
    {
        if (usedStepCount < MAX_STEPS) {
            steps[usedStepCount++] = step;
        }
    }

    void clearSteps() { usedStepCount = 0; }

    void setCondition(Step& step, float condition)
    {
        step.condition = CLAMP(condition, 0.0f, 1.0f);
    }

    void setNote(Step& step, int note)
    {
        step.note = (uint8_t)CLAMP(note, 0, 127);
    }

    void setVelocity(Step& step, float velocity)
    {
        step.velocity = CLAMP(velocity, 0.0f, 1.0f);
    }

    void setPosition(Step& step, uint16_t pos)
    {
        step.position = CLAMP(pos, 0, sequenceLength - 1);
    }

    auto onStep()
    {
        std::array<Event, 8> eventQueue;
        size_t eventIdx = 0;
        processLogic([&](const Event& ev) {
            if (eventIdx < eventQueue.size()) eventQueue[eventIdx++] = ev;
        });
        return std::make_pair(eventQueue, eventIdx);
    }

    Event onStepMono()
    {
        Event lastEvent = { EventType::None, nullptr };
        processLogic([&](const Event& ev) { lastEvent = ev; });
        return lastEvent;
    }

private:
    template <typename F>
    void processLogic(F&& callback)
    {
        stepCounter++;
        if (stepCounter >= sequenceLength) stepCounter = 0;

        for (size_t i = 0; i < usedStepCount; ++i) {
            auto& s = steps[i];

            // Note Off
            if (s.activeCounter > 0 && --s.activeCounter == 0) {
                callback({ EventType::NoteOff, &s });
            }

            // Note On
            if (stepCounter == s.position && s.velocity > 0.0f) {
                if (s.condition >= 1.0f || (Noise::get() <= s.condition)) {
                    s.activeCounter = s.len;
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