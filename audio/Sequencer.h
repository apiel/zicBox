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

#include "helpers/clamp.h"
#include "audio/utils/noise.h"

#include <cstdint>
#include <functional>
#include <vector>

class Sequencer {
public:
    enum Key {
        position = 'p',
        note = 'n',
        velocity = 'v',
        len = 'l',
        condition = 'c'
    };

    struct Step {
        uint16_t position = 0;
        uint8_t note = 60;
        float velocity = 0.8f;
        uint16_t len = 1;
        float condition = 1.0f;
        uint8_t counter = 0;
    };

    std::vector<Step> steps;

protected:
    uint32_t stepCounter = 0;
    uint16_t stepCount = 64;

    std::function<void(const Step&)> noteOnCallback;
    std::function<void(const Step&)> noteOffCallback;

public:
    Sequencer(
        std::function<void(const Step&)> onNoteOn,
        std::function<void(const Step&)> onNoteOff)
        : noteOnCallback(onNoteOn)
        , noteOffCallback(onNoteOff)
    {
    }

    void setCondition(Step& step, float condition)
    {
        step.condition = CLAMP(condition, 0.0f, 1.0f);
    }

    void setNote(Step& step, int note)
    {
        step.note = CLAMP(note, 0, 127);
    }

    void setVelocity(Step& step, float velocity)
    {
        step.velocity = CLAMP(velocity, 0.0, 1.0);
    }

    void setStepCount(uint16_t value)
    {
        stepCount = CLAMP(value, 4, 8192);
    }

    uint16_t getStepCount() const { return stepCount; }

    void reset()
    {
        stepCounter = 0;
    }

    void onStep()
    {
        stepCounter++;

        // If we reach the end of the sequence, reset step counter
        if (stepCounter >= stepCount) {
            stepCounter = 0;
        }

        for (auto& step : steps) {
            if (step.counter) {
                step.counter--;
                if (step.counter == 0) {
                    noteOffCallback(step); // trigger note off
                }
            }

            if (step.len && stepCounter == step.position && step.velocity > 0.0f) {
                if (step.condition < 1.0f) {
                    float rnd = Noise::get();
                    if (rnd > step.condition) {
                        continue;
                    }
                }
                step.counter = step.len;
                noteOnCallback(step); // trigger note on
            }
        }
    }

    struct KeyValue {
        char key;
        uint16_t value;
    };
    void hydrate(const std::vector<std::vector<KeyValue>>& values)
    {
        steps.clear();
        for (auto& stepValues : values) {
            Step step;
            for (auto& kv : stepValues) {
                if (kv.key == Key::position) step.position = kv.value;
                else if (kv.key == Key::note) step.note = kv.value;
                else if (kv.key == Key::velocity) step.velocity = kv.value / 100.0f;
                else if (kv.key == Key::len) step.len = kv.value;
                else if (kv.key == Key::condition) step.condition = kv.value / 100.0f;
            }
            steps.push_back(step);
        }
    }
    std::vector<std::vector<KeyValue>> serialize() const
    {
        std::vector<std::vector<KeyValue>> result;
        for (auto& step : steps) {
            std::vector<KeyValue> kv;
            kv.push_back({ Key::position, step.position });
            kv.push_back({ Key::note, step.note });
            kv.push_back({ Key::velocity, (uint16_t)(step.velocity * 100) });
            kv.push_back({ Key::len, step.len });
            kv.push_back({ Key::condition, (uint16_t)(step.condition * 100) });
            result.push_back(kv);
        }
        return result;
    }
};
