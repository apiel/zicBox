#ifndef _UI_PIXEL_COMPONENT_SEQ_SYNTH_BAR_H_
#define _UI_PIXEL_COMPONENT_SEQ_SYNTH_BAR_H_

#include "helpers/midiNote.h"
#include "plugins/audio/stepInterface.h"
#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"
#include "utils/GroupColorComponent.h"

/*md
## SeqSynthBar

*/

class SeqSynthBarComponent : public GroupColorComponent {
protected:
    AudioPlugin* plugin = NULL;
    Step* steps = NULL;
    uint8_t stepCount = 32;

    KeypadLayout keypadLayout;

    Color background;
    Color selection;
    ToggleColor text;
    ToggleColor foreground;
    ToggleColor activeStep;

    uint8_t stepIndex = -1;

    uint8_t encoderId1 = -1;
    uint8_t encoderId2 = -1;
    uint8_t encoderId3 = -1;
    uint8_t encoderId4 = -1;

public:
    SeqSynthBarComponent(ComponentInterface::Props props)
        : GroupColorComponent(props, { { "TEXT_COLOR", &text }, { "FOREGROUND_COLOR", &foreground }, { "ACTIVE_STEP_COLOR", &activeStep } })
        , background(styles.colors.background)
        , selection(styles.colors.primary)
        , text(styles.colors.text, inactiveColorRatio)
        , foreground({ 0x40, 0x40, 0x40 }, inactiveColorRatio)
        , activeStep(styles.colors.primary, inactiveColorRatio)
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            return func;
        })
    {
        updateColors();
    }

    void render() override
    {
        if (updatePosition() && steps) {
            draw.filledRect(relativePosition, size, { background });

            int stepW = 4;
            int stepH = size.h - 8;

            int stepsW = stepCount * (stepW + 2 + 0.5); // 2 / 4 adding 2 pixel every 4 steps
            int nameW = size.w - stepsW - 5;
            int x = relativePosition.x + 1;
            draw.filledRect({ x, relativePosition.y + 8 }, { nameW, stepH }, { foreground.color });
            x += nameW + 4;

            for (int i = 0; i < stepCount; i++) {
                Step* step = &steps[i];
                printf("step %d enabled: %d velocity: %f\n", i, step->enabled, step->velocity);
                Color color = step->enabled ? darken(activeStep.color, 1.0 - step->velocity) : foreground.color;
                draw.filledRect({ x, relativePosition.y + 8 }, { stepW, stepH }, { color });
                x += stepW + 2;
                if (i % 4 == 3) {
                    x += 2;
                }
            }
        }
    }

    // void onEncoder(int id, int8_t direction) override
    // {
    // }

    // void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    // {
    //     if (isActive) {
    //         keypadLayout.onKey(id, key, state, now);
    //     }
    // }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        if (keypadLayout.config(key, value)) {
            return true;
        }

        /*md - `PLUGIN: plugin_name [get_steps_data_id]` set plugin target */
        if (strcmp(key, "PLUGIN") == 0) {
            plugin = &getPlugin(strtok(value, " "), track);
            stepCount = plugin->getValue("SELECTED_STEP")->props().max;

            char* getStepsDataId = strtok(NULL, " ");
            uint8_t dataId = plugin->getDataId(getStepsDataId != NULL ? getStepsDataId : "STEPS");
            steps = (Step*)plugin->data(dataId);
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            background = draw.getColor(value);
            return true;
        }

        /*md - `SELECTION_COLOR: color` is the selection color. */
        if (strcmp(key, "SELECTION_COLOR") == 0) {
            selection = draw.getColor(value);
            return true;
        }

        /*md - `TEXT_COLOR: color` is the color of the text. */
        /*md - `FOREGROUND_COLOR: color` is the foreground color. */
        /*md - `ACTIVE_STEP_COLOR: color` is the color of the active step. */
        return GroupColorComponent::config(key, value);
    }
};

#endif
