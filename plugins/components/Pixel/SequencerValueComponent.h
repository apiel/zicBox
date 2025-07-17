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
    Color barColor;

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
        , barColor(lighten(styles.colors.background, 0.5))
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

        /// Set the color of the bar.
        barColor = draw.getColor(config["barColor"], barColor); //eg: "#FF0000"

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

        /// Type: "STEP_SELECTION", "STEP_TOGGLE"
        std::string type = config.value("type", "STEP_SELECTION");
        if (type == "STEP_TOGGLE") {
            renderFn = std::bind(&SequencerValueComponent::renderStepToggle, this);
            onEncoderFn = std::bind(&SequencerValueComponent::onEncoderStepToggle, this, std::placeholders::_1);
        } else {
            renderFn = std::bind(&SequencerValueComponent::renderSelectedStep, this);
            onEncoderFn = std::bind(&SequencerValueComponent::onEncoderStepSelection, this, std::placeholders::_1);
        }

        /*md md_config_end */
    }

    void render()
    {
        draw.filledRect(relativePosition, size, { background });
        renderFn();
    }

    void onEncoder(int id, int8_t direction) override
    {
        // printf("[track %d group %d][%d] SequencerValueComponent onEncoder: %d %d\n", track, group, encoderId, id, direction);
        if (id == encoderId) {
            onEncoderFn(direction);
        }
    }

    void onContext(uint8_t index, float value) override
    {
        if (index == contextId) {
            renderNext();
        }
    }

protected:
    std::function<void()> renderFn = std::bind(&SequencerValueComponent::renderSelectedStep, this);
    std::function<void(int8_t direction)> onEncoderFn = std::bind(&SequencerValueComponent::onEncoderStepSelection, this, std::placeholders::_1);

    void renderSelectedStep()
    {
        if (contextId != 0) {
            int selectedStep = view->contextVar[contextId];

            int x = relativePosition.x + (size.w) * 0.5;
            int y = relativePosition.y;

            draw.textCentered({ x, y }, std::to_string(selectedStep + 1) + "/" + std::to_string(*stepCount), valueFontSize, { valueColor, .font = fontValue });
            y += valueFontSize + 2;
            draw.textCentered({ x, y }, "Steps", labelFontSize, { labelColor, .font = fontLabel });
        }
    }

    void onEncoderStepSelection(int8_t direction)
    {
        int selectedStep = view->contextVar[contextId];
        int newStep = selectedStep + direction;
        if (newStep >= 0 && newStep < *stepCount) {
            setContext(contextId, newStep);
            renderNext();
        }
    }

    void renderStepToggle()
    {
        Step* step = getSelectedStep();
        bool isOn = step != nullptr && step->enabled;

        int x = relativePosition.x + (size.w) * 0.5;
        int y = relativePosition.y + 4;

        Color bgBar = darken(barColor, 0.4);
        draw.filledRect({ x - 10, y }, { 20, 20 }, { bgBar });
        y = y + 10;
        draw.filledCircle({ x + 10, y }, 10, { isOn ? barColor : bgBar });
        draw.filledCircle({ x - 10, y }, 10, { isOn ? bgBar : barColor });
        y = y + 12;
        draw.textCentered({ x, y }, isOn ? "ON" : "OFF", labelFontSize, { labelColor, .font = fontLabel });

        // draw.filledRect({ x - 5, y }, { 10, 10 }, { bgBar });
        // y = y + 5;
        // draw.filledCircle({ x + 5, y }, 5, { isOn ? barColor : bgBar });
        // draw.filledCircle({ x - 5, y }, 5, { isOn ? bgBar : barColor });
        // y = y + 5;
        // draw.textCentered({ x, y }, isOn ? "ON" : "OFF", valueFontSize, { valueColor, .font = fontValue });
    }

    void onEncoderStepToggle(int8_t direction)
    {
        Step* step = getSelectedStep();
        if (direction < 0) {
            if (step != nullptr) {
                step->enabled = false;
                renderNextAndSetContext();
            }
        } else {
            if (step) {
                step->enabled = true;
                renderNextAndSetContext();
            } else if (contextId != 0) {
                // Create a step and push it to the end
                Step newStep;
                newStep.position = view->contextVar[contextId];
                newStep.enabled = true;
                newStep.len = 1;
                steps->push_back(newStep);
                renderNextAndSetContext();
            }
        }
    }

    // Set context to make other components render
    void renderNextAndSetContext()
    {
        renderNext();
        setContext(contextId, view->contextVar[contextId]);
    }

    Step* getSelectedStep()
    {
        if (contextId != 0 && steps != NULL) {
            int selectedStep = view->contextVar[contextId];
            for (auto& step : *steps) {
                if (selectedStep == step.position || (selectedStep >= step.position && selectedStep < step.position + step.len)) {
                    return &step;
                }
            }
        }
        return nullptr;
    }
};
