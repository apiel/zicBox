#pragma once

#include "plugins/audio/stepInterface.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

/*md
## SequencerValue

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/SequencerValue.png" />

*/

class SequencerValueComponent : public Component {
protected:
    Color background;
    Color labelColor;
    Color valueColor;
    Color unitColor;

    void* fontValue = NULL;
    void* fontLabel = NULL;
    int valueFontSize = 12;
    int labelFontSize = 8;

    AudioPlugin* plugin;
    std::vector<Step>* steps = NULL;

    int maxSteps = 64;
    uint16_t* stepCount;

    uint8_t contextId = 0;

    int8_t encoderId = -1;

public:
    SequencerValueComponent(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            
            return func; })
        , background(styles.colors.background)
        , labelColor(alpha(styles.colors.white, 0.3))
        , valueColor(alpha(styles.colors.white, 0.6))
        , unitColor(alpha(styles.colors.white, 0.5))
    {
        /*md md_config:Sequencer */
        nlohmann::json& config = props.config;

        /// The audio plugin sequencer.
        plugin = getPluginPtr(config, "audioPlugin", track); //eq: "audio_plugin_name"

        /// The data id to get steps from audio plugin sequencer.
        steps = (std::vector<Step>*)plugin->data(plugin->getDataId(config.value("stepsDataId", "STEPS"))); //eg: "STEPS"

        /// The data id to get steps count from audio plugin sequencer.
        stepCount = (uint16_t*)plugin->data(plugin->getDataId(config.value("stepCountDataId", "STEP_COUNT"))); //eg: "STEP_COUNT"

        /// The maximum number of steps.
        maxSteps = config.value("maxSteps", maxSteps); //eg: 64

        /// The background color of the text.
        background = draw.getColor(config["bgColor"], background); //eg: "#000000"

        /// The color of the text.
        labelColor = draw.getColor(config["labelColor"], labelColor); //eg: "#FFFFFF"

        /// The color of the value.
        valueColor = draw.getColor(config["valueColor"], valueColor); //eg: "#FFFFFF"

        /// The color of the unit.
        unitColor = draw.getColor(config["unitColor"], unitColor); //eg: "#FFFFFF"

        /// Set context id shared between components to show selected step, must be different than 0.
        contextId = config.value("contextId", contextId); //eg: 10

        /// The font of the value. Default is null.
        if (config.contains("fontValue")) {
            fontValue = draw.getFont(config["fontValue"].get<std::string>().c_str()); //eg: "PoppinsLight_8"
            int fontSize = draw.getDefaultFontSize(fontValue);
            if (fontSize > 0) {
                valueFontSize = fontSize;
            }
        }

        /// The font of the label. Default is null.
        if (config.contains("fontLabel")) {
            fontLabel = draw.getFont(config["fontLabel"].get<std::string>().c_str()); //eg: "PoppinsLight_8"
            int fontSize = draw.getDefaultFontSize(fontLabel);
            if (fontSize > 0) {
                labelFontSize = fontSize;
            }
        }

        /// The encoder id that will interract with this component.
        /*md   encoderId={0} */
        encoderId = config.value("encoderId", encoderId);

        /*md md_config_end */
    }

    void render()
    {
        draw.filledRect(relativePosition, size, { background });

        if (contextId != 0 && steps != NULL) {
            int selectedStep = view->contextVar[contextId];

            int x = relativePosition.x + (size.w) * 0.5;
            int y = relativePosition.y;

            draw.textCentered({ x, y }, std::to_string(selectedStep + 1) + "/" + std::to_string(*stepCount), valueFontSize, { valueColor, .font = fontValue });
            y += valueFontSize + 2;
            draw.textCentered({ x, y }, "Steps", labelFontSize, { labelColor, .font = fontLabel });
        }
    }

    void onEncoder(int id, int8_t direction) override
    {
        // printf("[track %d group %d][%d] SequencerValueComponent onEncoder: %d %d\n", track, group, encoderId, id, direction);
        if (id == encoderId) {
            int selectedStep = view->contextVar[contextId];
            int newStep = selectedStep + direction;
            if (newStep >= 0 && newStep < *stepCount) {
                setContext(contextId, newStep);
                renderNext();
            }
        }
    }
};
