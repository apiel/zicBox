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
    AudioPlugin* seqPlugin = NULL;
    Step* steps = NULL;
    uint8_t stepCount = 32;

    AudioPlugin* synthPlugin = NULL;

    KeypadLayout keypadLayout;

    Color background;
    Color selection;
    ToggleColor text;
    ToggleColor foreground;
    ToggleColor activeStep;
    ToggleColor nameColor;
    ToggleColor label;

    uint8_t stepIndex = -1;

    ValueInterface* valStart = NULL;
    ValueInterface* valEnd = NULL;
    ValueInterface* valBrowser = NULL;
    ValueInterface* valVolume = NULL;
    ValueInterface* seqStatus;

public:
    SeqSynthBarComponent(ComponentInterface::Props props)
        : GroupColorComponent(props, { { "TEXT_COLOR", &text }, { "FOREGROUND_COLOR", &foreground }, { "ACTIVE_STEP_COLOR", &activeStep }, { "NAME_COLOR", &nameColor }, { "LABEL_COLOR", &label } })
        , background(styles.colors.background)
        , selection(styles.colors.primary)
        , text(styles.colors.text, inactiveColorRatio)
        , label(darken(styles.colors.text, 0.5), inactiveColorRatio)
        , foreground({ 0x40, 0x40, 0x40 }, inactiveColorRatio)
        , activeStep(styles.colors.primary, inactiveColorRatio)
        , nameColor(styles.colors.primary, inactiveColorRatio)
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            return func;
        })
    {
        updateColors();
    }

    int renderPct(ValueInterface* val, int x)
    {
        x = draw.text({ x, relativePosition.y }, val->label(), 8, { label.color });
        x = draw.text({ x + 4, relativePosition.y }, std::to_string((int)val->get()), 8, { text.color });
        x = draw.text({ x, relativePosition.y }, "%", 8, { label.color });
        return x;
    }

    void renderEncoders()
    {
        int x = relativePosition.x;

        // renderPct(someVal, x);
        x = renderPct(valVolume, x);
        x = renderPct(valStart, x + 15);
        x = renderPct(valEnd, x + 15);
    }

    void render() override
    {
        if (updatePosition() && steps && seqPlugin) {
            draw.filledRect(relativePosition, size, { background });

            renderEncoders();

            int stepW = 4;
            int stepH = size.h - 8;

            int stepsW = stepCount * (stepW + 2 + 0.5); // 2 / 4 adding 2 pixel every 4 steps
            int nameW = size.w - stepsW - 5;
            int x = relativePosition.x + 1;
            Color color = seqStatus->get() == 1 ? darken(nameColor.color, 0.5) : foreground.color;
            draw.filledRect({ x, relativePosition.y + 8 }, { nameW, stepH }, { color });
            if (seqStatus->get() == 1) {
                draw.filledRect({ x, relativePosition.y + 8 }, { (int)(nameW * valVolume->pct()), stepH }, { nameColor.color });
            }
            draw.text({ x + 2, relativePosition.y + 8 }, valBrowser->string(), 8, { text.color, .maxWidth = (nameW - 4) });
            x += nameW + 4;

            for (int i = 0; i < stepCount; i++) {
                Step* step = &steps[i];
                color = step->enabled ? darken(activeStep.color, 1.0 - step->velocity) : foreground.color;
                draw.filledRect({ x, relativePosition.y + 8 }, { stepW, stepH }, { color });
                x += stepW + 2;
                if (i % 4 == 3) {
                    x += 2;
                }
            }
        }
    }

    void onEncoder(int id, int8_t direction) override
    {
        if (active) {
            if (id == 0) {
                valVolume->increment(direction);
            } else if (id == 1) {
                valStart->increment(direction);
            } else if (id == 2) {
                valEnd->increment(direction);
            } else if (id == 3) {
                valBrowser->increment(direction);
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
            seqPlugin = &getPlugin(strtok(value, " "), track);
            stepCount = seqPlugin->getValue("SELECTED_STEP")->props().max;
            seqStatus = watch(seqPlugin->getValue("STATUS"));

            char* getStepsDataId = strtok(NULL, " ");
            uint8_t dataId = seqPlugin->getDataId(getStepsDataId != NULL ? getStepsDataId : "STEPS");
            steps = (Step*)seqPlugin->data(dataId);
            return true;
        }

        /*md - `SYNTH_PLUGIN: plugin_name` set the plugin target for the synth. */
        if (strcmp(key, "SYNTH_PLUGIN") == 0) {
            synthPlugin = &getPlugin(value, track);
            // might want to make this configurable...
            valBrowser = watch(synthPlugin->getValue("BROWSER"));
            valEnd = watch(synthPlugin->getValue("END"));
            valStart = watch(synthPlugin->getValue("START"));
            // FIXME
            valVolume = watch(getPlugin("Volume", track).getValue("VOLUME"));
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
        /*md - `NAME_COLOR: color` is the color of the name. */
        /*md - `LABEL_COLOR: color` is the color of the label. */
        return GroupColorComponent::config(key, value);
    }
};

#endif
