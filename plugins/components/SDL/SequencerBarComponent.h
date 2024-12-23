#ifndef _UI_COMPONENT_SEQUENCER_H_
#define _UI_COMPONENT_SEQUENCER_H_

#include "../audio/stepInterface.h"
#include "../utils/color.h"
#include "../component.h"

#include <string>

/*md
## SDL SequencerBar

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/SequencerBar.png" />

Make minimal representation of the sequencer.
*/
class SequencerBarComponent : public Component {
protected:
    AudioPlugin& plugin;
    ValueInterface* selectedStep = watch(getPlugin("Sequencer", track).getValue("SELECTED_STEP"));
    ValueInterface* stepEnabled = watch(getPlugin("Sequencer", track).getValue("STEP_ENABLED"));
    ValueInterface* stepLength = watch(getPlugin("Sequencer", track).getValue("STEP_LENGTH"));
    ValueInterface* stepVelocity = watch(getPlugin("Sequencer", track).getValue("STEP_VELOCITY"));
    ValueInterface* stepNote = watch(getPlugin("Sequencer", track).getValue("STEP_NOTE"));
    ValueInterface* stepCondition = watch(getPlugin("Sequencer", track).getValue("STEP_CONDITION"));

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
        draw.filledRect({ x, stepPosition.y }, stepSize, { color });

        int sel = selectedStep->get();
        if (index == sel) {
            draw.filledRect({ x, stepPosition.y - 3 }, { stepSize.w, 2 }, { colors.stepBackground });
        }

        if (index == *stepCounter) {
            draw.filledRect({ x, stepPosition.y - 3 }, { stepSize.w, 2 }, { colors.activePosition });
        } else if (steps[index].counter) {
            int w = stepSize.w * (steps[index].counter / (float)steps[index].len);
            draw.filledRect({ x + stepSize.w - w, stepPosition.y - 3 }, { w, 2 }, { colors.activePosition });
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
        return Colors({ darken(color, 0.75),
            darken(color, 0.3),
            color,
            { 0x00, 0xb3, 0x00, 255 },
            darken(color, 0.3) });
    }

    const int margin;

    uint8_t stepCount;

public:
    SequencerBarComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.primary))
        , margin(styles.margin)
        , plugin(getPlugin("Sequencer", track))
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
            { colors.background });

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
        draw.text({ stepPosition.x, position.y }, info, 9, { colors.textInfo });

        draw.text({ stepPosition.x + 50, position.y }, stepEnabled->string().c_str(), 9, { colors.textInfo });
        draw.text({ stepPosition.x + 70, position.y }, stepNote->string().c_str(), 9, { colors.textInfo });

        snprintf(info, 24, "len: %2d", (int)stepLength->get());
        draw.text({ stepPosition.x + 90, position.y }, info, 9, { colors.textInfo });

        snprintf(info, 24, "velocity: %3d%%", (int)(stepVelocity->get()));
        draw.text({ stepPosition.x + 120, position.y }, info, 9, { colors.textInfo });

        snprintf(info, 24, "condition: %s", stepCondition->string());
        draw.text({ stepPosition.x + 180, position.y }, info, 9, { colors.textInfo });
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
        /*md - `COLOR: #999999` set component color */
        if (strcmp(key, "COLOR") == 0) {
            colors = getColorsFromColor(draw.getColor(value));
            return true;
        }

        return false;
    }
};

#endif
