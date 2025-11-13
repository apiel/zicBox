#pragma once

#include "audio.h"
#include "valueView.h"

#include "helpers/format.h"
#include "helpers/midiNote.h"

#include <string>

class SeqView : public ValueView {
protected:
    int16_t currentStep = 0;
    Sequencer::Step* currentStepPtr = nullptr;

    void setCurrentStep(int16_t value)
    {
        currentStep = CLAMP(value, 0, audio.seq.getStepCount() - 1);

        for (auto& step : audio.seq.steps) {
            if (step.position == currentStep) {
                currentStepPtr = &step;
                return;
            }
        }
        currentStepPtr = nullptr;
    }

    void addStep()
    {
        audio.seq.steps.push_back(Sequencer::Step());
        setCurrentStep(currentStep);
    }

public:
    SeqView(DrawInterface& draw)
        : ValueView(draw)
    {
        setCurrentStep(currentStep);
    }

    void render() override
    {
        draw.clear();

        renderBar(valuePos[0], (float)currentStep / ((float)audio.seq.getStepCount() - 1.0f));
        renderStringValue(valuePos[0], "Step", std::to_string(currentStep + 1));

        uint16_t len = currentStepPtr ? currentStepPtr->len : 0;
        renderBar(valuePos[1], (float)len / (float)audio.seq.getStepCount());
        renderStringValue(valuePos[1], "Len", len ? (len == audio.seq.getStepCount() ? "infinite" : std::to_string(len)) : "---");

        uint8_t note = len == 0 ? 0 : currentStepPtr->note;
        renderStringValue(valuePos[2], "Note", note ? MIDI_NOTES_STR[note] : "---");

        // draw.line
        draw.line({ valuePos[3].x + 7, valuePos[3].y + 5 }, { valuePos[3].x + 30, valuePos[3].y - 7 });
        draw.line({ valuePos[3].x + 7, valuePos[3].y + 6 }, { valuePos[3].x + 30, valuePos[3].y - 6 });
        renderTextCentered(valuePos[3], std::to_string(audio.seq.getStepCount()), 6);

        float velocity = len == 0 ? -1.0f : currentStepPtr->velocity;
        renderBar(valuePos[4], velocity < 0.0f ? 0.0f : velocity);
        renderStringValue(valuePos[4], "Velocity", velocity < 0.0f ? "---" : std::to_string((int)(velocity * 100)) + "%");

        float condition = len == 0 ? -1.0f : currentStepPtr->condition;
        renderBar(valuePos[5], condition < 0.0f ? 0.0f : condition);
        renderStringValue(valuePos[5], "Cond.", condition < 0.0f ? "---" : std::to_string((int)(condition * 100)) + "%");
    }

    void onEncoder(int id, int8_t direction, uint64_t tick) override
    {
        if (id == 1) {
            setCurrentStep(currentStep + direction);
        } else if (id == 2) {
            if (currentStepPtr) {
                currentStepPtr->len = CLAMP(currentStepPtr->len + direction, 0, audio.seq.getStepCount());
            } else {
                addStep();
            }
        } else if (id == 3) {
            if (currentStepPtr) {
                if (currentStepPtr->len == 0) {
                    currentStepPtr->len = 1;
                } else {
                    currentStepPtr->note = CLAMP(currentStepPtr->note + direction, 12, 127);
                }
            } else {
                addStep();
            }
        } else if (id == 4) {
            uint16_t stepCount = audio.seq.getStepCount();

            if (direction > 0)
                stepCount = stepCount * 2;
            else if (direction < 0)
                stepCount = stepCount / 2;

            audio.seq.setStepCount(stepCount);
        } else if (id == 5) {
            if (currentStepPtr) {
                if (currentStepPtr->len == 0) {
                    currentStepPtr->len = 1;
                } else {
                    currentStepPtr->velocity = CLAMP(currentStepPtr->velocity + direction * 0.01f, 0.0f, 1.0f);
                }
            } else {
                addStep();
            }
        } else if (id == 6) {
            if (currentStepPtr) {
                if (currentStepPtr->len == 0) {
                    currentStepPtr->len = 1;
                } else {
                    currentStepPtr->condition = CLAMP(currentStepPtr->condition + direction * 0.01f, 0.1f, 1.0f);
                }
            } else {
                addStep();
            }
        }
        draw.renderNext();
    }

    bool onKey(uint16_t id, int key, int8_t state) override
    {
        return false;
    }
};