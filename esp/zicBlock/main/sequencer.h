#pragma once

#include "audio/Clock.h"
#include "audio/lookupTable.h"
#include "helpers/clamp.h"

#include <cstdint>
#include <functional>
#include <vector>

class Sequencer {

public:
    struct Step {
        uint16_t position = 0;
        uint8_t note = 60;
        float velocity = 0.8f;
        uint16_t len = 1; // len 0 is infinite?
        float condition = 1.0f;
        uint8_t counter = 0;
    };

    std::vector<Step> steps = {
        { 0, 60 },
        { 8, 60, 1.0f, 1, 0.85f },
        { 16, 60 },
        { 24, 60, 1.0f, 1, 0.85f },
        { 32, 60 },
        { 40, 60, 1.0f, 1, 0.85f },
        { 48, 60 },
        { 56, 60, 1.0f, 1, 0.85f },
    };

    Clock clock;

protected:
    LookupTable& lookupTable;

    uint16_t stepCounter = 0;
    bool isPlaying = false;
    uint16_t loopCounter = 0;
    uint16_t stepCount = 64;

    std::function<void(const Step&)> noteOnCallback;
    std::function<void(const Step&)> noteOffCallback;

    void onStep()
    {
        stepCounter++;

        // If we reach the end of the sequence, reset step counter
        if (stepCounter >= stepCount) {
            stepCounter = 0;
            loopCounter++;
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
                    float rnd = (lookupTable.getNoise() + 1.0f) / 2.0f;
                    if (rnd > step.condition) {
                        continue;
                    }
                }
                step.counter = step.len;
                noteOnCallback(step); // trigger note on
            }
        }
    }

public:
    Sequencer(
        int sampleRate,
        uint8_t channels,
        LookupTable& lookupTable,
        std::function<void(const Step&)> onNoteOn,
        std::function<void(const Step&)> onNoteOff)
        : clock(sampleRate, channels)
        , lookupTable(lookupTable)
        , noteOnCallback(onNoteOn)
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
