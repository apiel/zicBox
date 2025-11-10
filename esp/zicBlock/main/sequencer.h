#pragma once

#include "audio/Clock.h"
#include "helpers/clamp.h"

#include <cstdint>
#include <vector>

class Sequencer {
protected:
    Clock clock;

    uint16_t stepCounter = 0;
    bool isPlaying = false;
    uint16_t loopCounter = 0;
    uint16_t stepCount = 64;

    void onStep()
    {
        stepCounter++;

        // If we reach the end of the sequence, we reset the step counter
        if (stepCounter >= stepCount) {
            stepCounter = 0;
            loopCounter++;
        }

        for (auto& step : steps) {
            if (step.counter) {
                step.counter--;
                if (step.counter == 0) {
                    // trigger note off
                }
            }
            if (step.len && stepCounter == step.position && step.velocity > 0.0f) {
                step.counter = step.len;
                // trigger note on
            }
        }
    }

public:
    struct Step {
        float velocity = 0.8f;
        float condition = 1.0f;
        uint16_t position = 0;
        uint16_t len = 1; // len 0 is infinite?
        uint8_t counter = 0;
        uint8_t note = 60;
    };

    std::vector<Step> steps;

    Sequencer(int sampleRate, uint8_t channels)
        : clock(sampleRate, channels)
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

    void setStepCount(uint16_t stepCount)
    {
        this->stepCount = stepCount;
    }

    void next()
    {
        uint32_t clockValue = clock.getClock();
        if (clockValue != 0) {
            if (clockValue % 6 == 0) {
                onStep();
            }
            // on clock
        }
    }
};
