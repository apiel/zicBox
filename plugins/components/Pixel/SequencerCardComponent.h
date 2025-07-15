#pragma once

// #include "helpers/midiNote.h"
// #include "host/constants.h" // TODO instead load from plugin seq
#include "plugins/audio/stepInterface.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

// #include <string>

/*md
## SequencerCard

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/sequencerCard.png" />

*/

class SequencerCardComponent : public Component {
protected:
    Color background;
    Color activeStepColor;
    Color stepBackground;
    Color inactiveStepColor;

    AudioPlugin* plugin;
    std::vector<Step>* steps = NULL;

    int stepPerRow = 8;
    int maxSteps = 64;
    uint16_t* stepCount;

    int stepWidth = 0;
    int stepHeight = 0;
    int rowCount = 0;

public:
    SequencerCardComponent(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            
            return func; })
        , background(styles.colors.background)
        , activeStepColor(styles.colors.primary)
        , stepBackground(alpha(styles.colors.primary, 0.2))
        , inactiveStepColor(alpha(styles.colors.primary, 0.07))
    {
        /*md md_config:Sequencer */
        nlohmann::json& config = props.config;

        /// The audio plugin sequencer.
        plugin = getPluginPtr(config, "audioPlugin", track); //eq: "audio_plugin_name"

        /// The data id to get steps from audio plugin sequencer.
        steps = (std::vector<Step>*)plugin->data(plugin->getDataId(config.value("stepsDataId", "STEPS"))); //eg: "STEPS"

        /// The data id to get steps count from audio plugin sequencer.
        stepCount = (uint16_t*)plugin->data(plugin->getDataId(config.value("stepCountDataId", "STEP_COUNT"))); //eg: "STEP_COUNT"

        /// The number of steps per row.
        stepPerRow = config.value("stepPerRow", stepPerRow); //eg: 8

        /// The maximum number of steps.
        maxSteps = config.value("maxSteps", maxSteps); //eg: 64

        /// The background color of the text.
        background = draw.getColor(config["bgColor"], background); //eg: "#000000"

        /// Color of the active step.
        activeStepColor = draw.getColor(config["activeStepColor"], activeStepColor); //eg: "#000000"

        /// Color of the step background.
        stepBackground = draw.getColor(config["stepBackground"], stepBackground); //eg: "#000000"

        /// Color of the inactive step.
        inactiveStepColor = draw.getColor(config["inactiveStepColor"], inactiveStepColor); //eg: "#000000"

        /*md md_config_end */

        resize();
    }

    void resize() override
    {
        stepWidth = size.w / stepPerRow;
        rowCount = maxSteps / stepPerRow;
        stepHeight = size.h / rowCount;
    }

    void render()
    {
        draw.filledRect(relativePosition, size, { background });

        for (int i = 0; i < rowCount; i++) {
            int y = relativePosition.y + i * stepHeight;
            for (int j = 0; j < stepPerRow; j++) {
                int x = relativePosition.x + j * stepWidth;
                Color color = i * stepPerRow + j < *stepCount ? stepBackground : inactiveStepColor;
                draw.filledRect({ x + 1, y + 1 }, { stepWidth - 2, stepHeight - 2 }, { color });
            }
        }

        if (steps != NULL) {
            for (const auto& step : *steps) {
                if (step.len && step.enabled) {
                    int stepRow = step.position / stepPerRow;
                    int stepCol = step.position % stepPerRow;
                    int x = relativePosition.x + stepCol * stepWidth + 1;
                    int y = relativePosition.y + stepRow * stepHeight;
                    draw.filledRect({ x, y }, { stepWidth - 2, stepHeight - 2 }, { activeStepColor });
                }
            }
        }
    }
};
