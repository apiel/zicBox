#pragma once

#include "helpers/midiNote.h"
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

    bool selectStepUsingLength = false;

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

        /// Type: "STEP_SELECTION", "STEP_TOGGLE", "STEP_NOTE", "STEP_VELOCITY", "STEP_CONDITION", "STEP_LENGTH"
        std::string type = config.value("type", "STEP_SELECTION");
        if (type == "STEP_TOGGLE") {
            renderFn = std::bind(&SequencerValueComponent::renderStepToggle, this);
            onEncoderFn = std::bind(&SequencerValueComponent::onEncoderStepToggle, this, std::placeholders::_1);
        } else if (type == "STEP_NOTE") {
            renderFn = std::bind(&SequencerValueComponent::renderStepNote, this);
            onEncoderFn = std::bind(&SequencerValueComponent::onEncoderStepNote, this, std::placeholders::_1);
        } else if (type == "STEP_VELOCITY") {
            renderFn = std::bind(&SequencerValueComponent::renderStepVelocity, this);
            onEncoderFn = std::bind(&SequencerValueComponent::onEncoderStepVelocity, this, std::placeholders::_1);
        } else if (type == "STEP_CONDITION") {
            renderFn = std::bind(&SequencerValueComponent::renderStepCondition, this);
            onEncoderFn = std::bind(&SequencerValueComponent::onEncoderStepCondition, this, std::placeholders::_1);
        } else if (type == "STEP_LENGTH") {
            renderFn = std::bind(&SequencerValueComponent::renderStepLength, this);
            onEncoderFn = std::bind(&SequencerValueComponent::onEncoderStepLength, this, std::placeholders::_1);
        } else if (type == "STEP_MOTION") {
            renderFn = std::bind(&SequencerValueComponent::renderStepMotion, this);
            onEncoderFn = std::bind(&SequencerValueComponent::onEncoderStepMotion, this, std::placeholders::_1);
        } else {
            renderFn = std::bind(&SequencerValueComponent::renderSelectedStep, this);
            onEncoderFn = std::bind(&SequencerValueComponent::onEncoderStepSelection, this, std::placeholders::_1);
        }

        /// Select step using as well the length, meaning the step will be selected not only from his starting position but also any following steps covered by his length.
        selectStepUsingLength = config.value("selectStepUsingLength", selectStepUsingLength);

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
        if (newStep < 0) {
            newStep = *stepCount - 1;
        } else if (newStep >= *stepCount) {
            newStep = 0;
        }
        setContext(contextId, newStep);
        renderNext();
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

    void renderStepNote()
    {
        Step* step = getSelectedStep();
        int x = relativePosition.x + (size.w) * 0.5;
        if (step) {
            int y = relativePosition.y + 4;
            int note = step->note;
            draw.textCentered({ x, y }, MIDI_NOTES_STR[note], valueFontSize, { valueColor, .font = fontValue });
        } else {
            int y = relativePosition.y + 4;
            draw.textCentered({ x, y }, "---", valueFontSize, { labelColor, .font = fontValue });
        }
    }

    void onEncoderStepNote(int8_t direction)
    {
        Step* step = getSelectedStep();
        if (step) {
            step->note += direction;
            if (step->note > 250) { // when it goes -1 it becomes 255
                step->note = 127;
            } else if (step->note > 127) {
                step->note = 0;
            }
            renderNext();
        }
    }

    void renderStepCondition()
    {
        Step* step = getSelectedStep();
        int x = relativePosition.x + (size.w) * 0.5;
        int y = relativePosition.y;

        std::string condition = step ? stepConditions[step->condition].name : "---";
        if (condition == "---") {
            draw.textCentered({ x, y }, "---", valueFontSize, { labelColor, .font = fontValue });
        } else {
            draw.textCentered({ x, y }, condition, valueFontSize, { valueColor, .font = fontValue });
        }
        y += valueFontSize + 2;
        draw.textCentered({ x, y }, "Condition", labelFontSize, { labelColor, .font = fontLabel });
    }

    void onEncoderStepCondition(int8_t direction)
    {
        Step* step = getSelectedStep();
        if (step) {
            step->setCondition(step->condition + direction);
            renderNext();
        }
    }

    void renderStepMotion()
    {
        Step* step = getSelectedStep();
        int x = relativePosition.x + (size.w) * 0.5;
        int y = relativePosition.y;

        std::string motion = step ? stepMotions[step->motion].name : "---";
        if (motion == "---") {
            draw.textCentered({ x, y }, "---", valueFontSize, { labelColor, .font = fontValue });
        } else {
            draw.textCentered({ x, y }, motion, valueFontSize, { valueColor, .font = fontValue });
        }
        y += valueFontSize + 2;
        draw.textCentered({ x, y }, "motion", labelFontSize, { labelColor, .font = fontLabel });
    }

    void onEncoderStepMotion(int8_t direction)
    {
        Step* step = getSelectedStep();
        if (step) {
            step->setMotion(step->motion + direction);
            renderNext();
        }
    }

    void renderStepLength()
    {
        Step* step = getSelectedStep();
        int x = relativePosition.x + (size.w) * 0.5;
        int y = relativePosition.y;

        if (step) {
            draw.textCentered({ x, y }, std::to_string(step->len), valueFontSize, { valueColor, .font = fontValue });
        } else {
            draw.textCentered({ x, y }, "---", valueFontSize, { labelColor, .font = fontValue });
        }
        y += valueFontSize + 2;
        draw.textCentered({ x, y }, "Length", labelFontSize, { labelColor, .font = fontLabel });
    }

    void onEncoderStepLength(int8_t direction)
    {
        Step* step = getSelectedStep();
        if (step) {
            step->setLength(step->len + direction);
            // renderNext();
            renderNextAndSetContext();
        }
    }

    void renderStepVelocity()
    {
        Step* step = getSelectedStep();
        int x = relativePosition.x + (size.w) * 0.5;
        int y = relativePosition.y;
        if (step) {
            draw.filledRect({ relativePosition.x, y }, { size.w, 1 }, { darken(barColor, 0.4) });
            draw.filledRect({ relativePosition.x, y }, { (int)(size.w * step->velocity), 3 }, { barColor });

            draw.textCentered({ x, y }, std::to_string((int)(step->velocity * 100)) + "%", valueFontSize, { valueColor, .font = fontValue });
        } else {
            draw.textCentered({ x, y }, "---", valueFontSize, { labelColor, .font = fontValue });
        }
        y += valueFontSize + 2;
        draw.textCentered({ x, y }, "Velocity", labelFontSize, { labelColor, .font = fontLabel });
    }

    void onEncoderStepVelocity(int8_t direction)
    {
        Step* step = getSelectedStep();
        if (step) {
            step->setVelocity(step->velocity + direction * 0.01);
            renderNext();
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
                if (selectedStep == step.position) {
                    return &step;
                } else if (selectStepUsingLength && selectedStep >= step.position && selectedStep < step.position + step.len) {
                    return &step;
                }
            }
        }
        return nullptr;
    }
};
