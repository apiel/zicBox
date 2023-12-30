#ifndef _UI_COMPONENT_SEQUENCER_H_
#define _UI_COMPONENT_SEQUENCER_H_

#include "../audio/stepInterface.h"
#include "component.h"

#include <string>

class SequencerBarComponent : public Component {
protected:
    AudioPlugin& plugin;
    ValueInterface* selectedStep = val(getPlugin({ "Sequencer" }).getValue("SELECTED_STEP"));
    ValueInterface* stepEnabled = val(getPlugin({ "Sequencer" }).getValue("STEP_ENABLED"));
    ValueInterface* stepLength = val(getPlugin({ "Sequencer" }).getValue("STEP_LENGTH"));
    ValueInterface* stepVelocity = val(getPlugin({ "Sequencer" }).getValue("STEP_VELOCITY"));
    ValueInterface* stepNote = val(getPlugin({ "Sequencer" }).getValue("STEP_NOTE"));
    ValueInterface* stepCondition = val(getPlugin({ "Sequencer" }).getValue("STEP_CONDITION"));
    ValueInterface* pattern = val(getPlugin({ "Sequencer" }).getValue("PATTERN"));

    Point stepPosition;
    Size stepSize;
    int stepMarginTop = 18;

    Step* steps;
    uint8_t* stepCounter;
    uint8_t previousStepCounter = 0;

    int debounceSelectedStep = 0;
    float previousSelectedStep = 0.0;

    void renderStep(uint8_t index)
    {
        Color color = colors.stepBackground;
        if (steps[index].enabled) {
            color = colors.stepEnabled;
        } else if (index % 4 != 0) {
            color.a = 150;
        }

        int x = stepPosition.x + (index * (stepSize.w + margin));
        draw.filledRect({ x, stepPosition.y }, stepSize, color);

        int sel = selectedStep->get();
        if (index == sel) {
            draw.filledRect({ x, stepPosition.y - 3 }, { stepSize.w, 2 }, colors.stepBackground);
        }

        if (index == *stepCounter) {
            draw.filledRect({ x, stepPosition.y - 3 }, { stepSize.w, 2 }, colors.activePosition);
        } else if (steps[index].counter) {
            int w = stepSize.w * (steps[index].counter / (float)steps[index].len);
            draw.filledRect({ x + stepSize.w - w, stepPosition.y - 3 }, { w, 2 }, colors.activePosition);
        }
    }

    struct Colors {
        Color background;
        Color stepBackground;
        Color stepEnabled;
        Color activePosition;
        Color textInfo;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ draw.darken(color, 0.75),
            draw.darken(color, 0.3),
            color,
            styles.colors.on,
            draw.darken(color, 0.3) });
    }

    const int margin;

    uint8_t stepCount;

public:
    SequencerBarComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
        , margin(styles.margin)
        , plugin(getPlugin({ "Sequencer" }))
    {
        stepCount = selectedStep->props().max;

        stepSize = {
            (int)(((props.size.w) / (float)stepCount) - margin),
            props.size.h - (stepMarginTop + margin * 2)
        };

        stepPosition = {
            position.x + margin + (int)((size.w - ((stepSize.w + margin) * stepCount)) * 0.5),
            position.y + stepMarginTop
        };

        steps = (Step*)plugin.data(0);
        stepCounter = (uint8_t*)plugin.data(1);

        jobRendering = [this](unsigned long now) {
            if (previousStepCounter != *stepCounter || previousSelectedStep != selectedStep->get()) {
                previousStepCounter = *stepCounter;
                previousSelectedStep = selectedStep->get();
                renderNext();
            }
        };
    }

    void render()
    {
        draw.filledRect(
            { position.x + margin, position.y + margin },
            { size.w - 2 * margin, size.h - 2 * margin },
            colors.background);

        for (int i = 0; i < stepCount; i++) {
            renderStep(i);
            // printf("step %d, note %d, enabled %d, len %d\n", i, steps[i].note, steps[i].enabled, steps[i].len);
        }

        char info[24];
        int stepIndex = selectedStep->get();
        if (stepIndex > stepCount) {
            stepIndex = stepCount;
        }
        snprintf(info, 24, "Step: %.2d/%d", stepIndex, stepCount);
        draw.text({ stepPosition.x, position.y }, info, colors.textInfo, 9);

        draw.text({ stepPosition.x + 50, position.y }, stepEnabled->string().c_str(), colors.textInfo, 9);
        draw.text({ stepPosition.x + 70, position.y }, stepNote->string().c_str(), colors.textInfo, 9);

        snprintf(info, 24, "len: %2d", (int)stepLength->get());
        draw.text({ stepPosition.x + 90, position.y }, info, colors.textInfo, 9);

        snprintf(info, 24, "velocity: %3d%%", (int)(stepVelocity->get()));
        draw.text({ stepPosition.x + 120, position.y }, info, colors.textInfo, 9);

        snprintf(info, 24, "condition: %s", stepCondition->string());
        draw.text({ stepPosition.x + 180, position.y }, info, colors.textInfo, 9);
    }

    void onMotion(MotionInterface& motion)
    {
        int index = (motion.position.x - stepPosition.x) / (stepSize.w + margin);

        if (debounceSelectedStep != index) {
            debounceSelectedStep = index;
            selectedStep->set(index);
        }
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "COLOR") == 0) {
            colors = getColorsFromColor(draw.getColor(value));
            return true;
        }

        return false;
    }
};

#endif
