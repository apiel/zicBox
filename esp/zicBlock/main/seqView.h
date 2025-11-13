#pragma once

#include "audio.h"
#include "valueView.h"

#include "helpers/format.h"

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

public:
    SeqView(DrawInterface& draw)
        : ValueView(draw)
    {
    }

    void render() override
    {
        draw.clear();

        renderBar(valuePos[0], (float)currentStep / ((float)audio.seq.getStepCount() - 1.0f));
        renderStringValue(valuePos[0], "Step", std::to_string(currentStep + 1));

        // renderBar(valuePos[1], (float)audio.fx2.getIndex() / ((float)audio.fx2.count - 1.0f));
        // renderStringValue(valuePos[1], "Fx2", audio.fx2.getShortName());

        // renderBar(valuePos[2], (float)audio.fx3.getIndex() / ((float)audio.fx3.count - 1.0f));
        // renderStringValue(valuePos[2], "Fx3", audio.fx3.getShortName());

        // renderBar(valuePos[3], (float)audio.fx1Amount);
        // renderStringValue(valuePos[3], "Fx1", std::to_string((int)(audio.fx1Amount * 100)) + "%");

        // renderBar(valuePos[4], (float)audio.fx2Amount);
        // renderStringValue(valuePos[4], "Fx2", std::to_string((int)(audio.fx2Amount * 100)) + "%");

        // renderBar(valuePos[5], (float)audio.fx3Amount);
        // renderStringValue(valuePos[5], "Fx3", std::to_string((int)(audio.fx3Amount * 100)) + "%");
    }

    void onEncoder(int id, int8_t direction, uint64_t tick) override
    {
        if (id == 1) {
            setCurrentStep(currentStep + direction);

            // } else if (id == 2) {
            //     audio.fx2.set(audio.fx2.getIndex() + (direction > 0 ? 1 : -1));
            // } else if (id == 3) {
            //     audio.fx3.set(audio.fx3.getIndex() + (direction > 0 ? 1 : -1));
            // } else if (id == 4) {
            //     audio.setFx1Amount(audio.fx1Amount + direction * 0.01f);
            // } else if (id == 5) {
            //     audio.setFx2Amount(audio.fx2Amount + direction * 0.01f);
            // } else if (id == 6) {
            //     audio.setFx3Amount(audio.fx3Amount + direction * 0.01f);
        }
        draw.renderNext();
    }

    bool onKey(uint16_t id, int key, int8_t state) override
    {
        return false;
    }
};