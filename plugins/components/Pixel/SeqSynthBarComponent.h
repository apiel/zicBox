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
    Step* steps = NULL;
    uint8_t stepCount = 32;

    KeypadLayout keypadLayout;

    Color background;
    ToggleColor selection;
    Color textColor;
    Color foreground;
    Color activeStepColor;
    Color nameColor;
    Color labelColor;

    uint8_t stepIndex = -1;

    ValueInterface* valName = NULL;
    ValueInterface* valVolume = NULL;
    ValueInterface* seqStatus = NULL;

public:
    SeqSynthBarComponent(ComponentInterface::Props props)
        : GroupColorComponent(props, { { "SELECTION_COLOR", &selection } }, 1.0)
        , background(styles.colors.background)
        , selection(styles.colors.white, inactiveColorRatio)
        , textColor(styles.colors.text)
        , labelColor(darken(styles.colors.text, 0.5))
        , foreground({ 0x40, 0x40, 0x40 })
        , activeStepColor(styles.colors.primary)
        , nameColor(styles.colors.primary)
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            return func;
        })
    {
        // setInactiveColorRatio(1.0);
        updateColors();
    }

    void render() override
    {
        if (updatePosition() && steps) {
            draw.filledRect(relativePosition, size, { background });

            int stepW = 4;
            int stepH = size.h;

            int textY = (size.h - 8) * 0.5 + relativePosition.y;

            int stepsW = stepCount * (stepW + 2 + 0.5); // 2 / 4 adding 2 pixel every 4 steps
            int nameW = size.w - stepsW - 5;
            int x = relativePosition.x + 1;
            bool showVolume = seqStatus != NULL && seqStatus->get() == 1 && valVolume != NULL;
            Color color = showVolume ? darken(nameColor, 0.5) : foreground;
            draw.filledRect({ x, relativePosition.y }, { nameW, stepH }, { color });
            if (showVolume) {
                draw.filledRect({ x, relativePosition.y }, { (int)(nameW * valVolume->pct()), stepH }, { nameColor });
            }
            if (valName != NULL) {
                draw.text({ x + 2, textY }, valName->string(), 8, { textColor, .maxWidth = (nameW - 4) });
            }
            draw.rect({ x, relativePosition.y }, { nameW, stepH }, { selection.color });
            x += nameW + 4;

            for (int i = 0; i < stepCount; i++) {
                Step* step = &steps[i];
                color = step->enabled ? darken(activeStepColor, 1.0 - step->velocity) : foreground;
                draw.filledRect({ x, relativePosition.y }, { stepW, stepH }, { color });
                x += stepW + 2;
                if (i % 4 == 3) {
                    x += 2;
                }
            }
        }
    }

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

        /*md - `SEQ_PLUGIN: plugin_name [get_steps_data_id]` set plugin target for sequencer */
        if (strcmp(key, "SEQ_PLUGIN") == 0) {
            AudioPlugin* seqPlugin = NULL;

            seqPlugin = &getPlugin(strtok(value, " "), track);
            stepCount = seqPlugin->getValue("SELECTED_STEP")->props().max;
            seqStatus = watch(seqPlugin->getValue("STATUS"));

            char* getStepsDataId = strtok(NULL, " ");
            uint8_t dataId = seqPlugin->getDataId(getStepsDataId != NULL ? getStepsDataId : "STEPS");
            steps = (Step*)seqPlugin->data(dataId);
            return true;
        }

        /*md - `NAME_PLUGIN: plugin_name value_key` set the plugin target to be used for the name. */
        if (strcmp(key, "NAME_PLUGIN") == 0) {
            valName = watch(getPlugin(strtok(value, " "), track).getValue(strtok(NULL, " ")));
            return true;
        }

        /*md - `VOLUME_PLUGIN: plugin_name value_key` is used for the volume bar (but can be any else). */
        if (strcmp(key, "VOLUME_PLUGIN") == 0) {
            valVolume = watch(getPlugin(strtok(value, " "), track).getValue(strtok(NULL, " ")));
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            background = draw.getColor(value);
            return true;
        }

        /*md - `TEXT_COLOR: color` is the color of the text. */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            textColor = draw.getColor(value);
            return true;
        }

        /*md - `FOREGROUND_COLOR: color` is the foreground color. */
        if (strcmp(key, "FOREGROUND_COLOR") == 0) {
            foreground = draw.getColor(value);
            return true;
        }

        /*md - `ACTIVE_STEP_COLOR: color` is the color of the active step. */
        if (strcmp(key, "ACTIVE_STEP_COLOR") == 0) {
            activeStepColor = draw.getColor(value);
            return true;
        }

        /*md - `NAME_COLOR: color` is the color of the name. */
        if (strcmp(key, "NAME_COLOR") == 0) {
            nameColor = draw.getColor(value);
            return true;
        }

        /*md - `LABEL_COLOR: color` is the color of the label. */
        if (strcmp(key, "LABEL_COLOR") == 0) {
            labelColor = draw.getColor(value);
            return true;
        }

        /*md - `SELECTION_COLOR: color` is the selection color. */
        return GroupColorComponent::config(key, value);
    }
};

#endif
