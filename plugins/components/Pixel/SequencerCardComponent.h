#pragma once

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
    Color stepLengthColor;
    Color stepBackground;
    Color inactiveStepColor;
    Color stepSelectedColor;
    Color rowsSelectionColor;
    Color playingColor;

    AudioPlugin* plugin;
    std::vector<Step>* steps = NULL;

    int stepPerRow = 8;
    int maxSteps = 64;
    uint16_t* stepCount;
    uint16_t* stepCounter;
    bool* isPlaying;

    int stepWidth = 0;
    int stepHeight = 0;
    int rowCount = 0;

    uint8_t contextId = 0;
    uint8_t rowsSelection = 0;

    bool renderPlayingStep = false;
    int lastPlayingStep = 0;
    std::function<void(bool)> onPlayStep = [this](bool isPlaying) {
        // renderPlayingStep = true;
        // renderNext();
        if (isPlaying) {
            jobRendering = [this](unsigned long now) {
                renderPlayingStep = true;
                renderNext();
            };
        } else {
            renderNext();
            jobRendering = [this](unsigned long now) { };
        }
    };

public:
    SequencerCardComponent(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            
            return func; })
        , background(styles.colors.background)
        , activeStepColor(styles.colors.primary)
        , stepLengthColor(alpha(styles.colors.primary, 0.3))
        , stepBackground(alpha(styles.colors.primary, 0.2))
        , inactiveStepColor(alpha(styles.colors.primary, 0.07))
        , stepSelectedColor(styles.colors.white)
        , rowsSelectionColor(lighten(styles.colors.background, 1.0))
        , playingColor(styles.colors.white)
    {
        jobRendering = [this](unsigned long now) { }; // Initialize with empty job to register callback

        /*md md_config:Sequencer */
        nlohmann::json& config = props.config;

        /// The audio plugin sequencer.
        plugin = getPluginPtr(config, "audioPlugin", track); //eq: "audio_plugin_name"

        if (plugin) {
            /// The data id to get steps from audio plugin sequencer.
            steps = (std::vector<Step>*)plugin->data(plugin->getDataId(config.value("stepsDataId", "STEPS"))); //eg: "STEPS"

            /// The data id to get step counter (current playing step) from audio plugin sequencer.
            stepCounter = (uint16_t*)plugin->data(plugin->getDataId(config.value("stepCounterDataId", "STEP_COUNTER"))); //eg: "STEP_COUNTER"

            /// The data id to get is playing state from audio plugin sequencer.
            isPlaying = (bool*)plugin->data(plugin->getDataId(config.value("isPlayingDataId", "IS_PLAYING"))); //eg: "IS_PLAYING"

            // TODO should this be enable by default?
            /// Data id to register playing sequencer event callback.
            plugin->data(plugin->getDataId(config.value("eventCallbackDataId", "REGISTER_CALLBACK")), &onPlayStep);

            /// The data id to get steps count from audio plugin sequencer.
            stepCount = (uint16_t*)plugin->data(plugin->getDataId(config.value("stepCountDataId", "STEP_COUNT"))); //eg: "STEP_COUNT"
        }

        /// The number of steps per row.
        stepPerRow = config.value("stepPerRow", stepPerRow); //eg: 8

        /// The maximum number of steps.
        maxSteps = config.value("maxSteps", maxSteps); //eg: 64

        /// The background color of the text.
        background = draw.getColor(config["bgColor"], background); //eg: "#000000"

        /// Color of the active step.
        activeStepColor = draw.getColor(config["activeStepColor"], activeStepColor); //eg: "#000000"

        /// Color of the step length.
        stepLengthColor = draw.getColor(config["stepLengthColor"], stepLengthColor); //eg: "#000000"

        /// Color of the step background.
        stepBackground = draw.getColor(config["stepBackground"], stepBackground); //eg: "#000000"

        /// Color of the inactive step.
        inactiveStepColor = draw.getColor(config["inactiveStepColor"], inactiveStepColor); //eg: "#000000"

        /// Color of the selected step.
        stepSelectedColor = draw.getColor(config["stepSelectedColor"], stepSelectedColor); //eg: "#000000"

        /// Color of the rows selection.
        rowsSelectionColor = draw.getColor(config["rowsSelectionColor"], rowsSelectionColor); //eg: "#000000"

        /// Set how many rows are selected, for example to show which rows are currently displayed on the keypad grid.
        rowsSelection = config.value("rowsSelection", rowsSelection); //eg: 2

        /// Set context id shared between components to show selected step, must be different than 0.
        contextId = config.value("contextId", contextId); //eg: 10

        /*md md_config_end */

        resize();
    }

    void resize() override
    {
        stepWidth = size.w / stepPerRow;
        rowCount = maxSteps / stepPerRow;
        stepHeight = size.h / rowCount;
    }

    void renderStepAtPos(int pos, Color color)
    {
        int row = pos / stepPerRow;
        int col = pos % stepPerRow;
        int x = relativePosition.x + col * stepWidth;
        int y = relativePosition.y + row * stepHeight;
        draw.filledRect({ x + 1, y + 1 }, { stepWidth - 2, stepHeight - 2 }, { color });
    }

    void render() override
    {
        if (renderPlayingStep) {
            renderPlayingStep = false;
            renderWhichStepIsPlaying(lastPlayingStep, true);
            lastPlayingStep = (*stepCounter);
            renderWhichStepIsPlaying(lastPlayingStep);
            return;
        }

        draw.filledRect(relativePosition, size, { background });

        // Render scrollbar..
        if (contextId != 0 && rowsSelection > 0) {
            int selectedStep = view->contextVar[contextId];

            // Get which group of rows the selected step belongs to
            int stepGroup = selectedStep / (stepPerRow * rowsSelection);
            int baseRow = stepGroup * rowsSelection;

            int y = relativePosition.y + baseRow * stepHeight;
            // draw.filledRect({ relativePosition.x, y }, { size.w, stepHeight * rowsSelection }, { rowsSelectionColor });

            draw.filledRect({ relativePosition.x + size.w + 1, relativePosition.y }, { 2, size.h }, { background });
            draw.filledRect({ relativePosition.x + size.w + 1, y }, { 2, stepHeight * rowsSelection }, { rowsSelectionColor });
        }

        // Render steps background
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
                if (step.len && step.enabled && step.position < *stepCount) {
                    // Render the main (first) step
                    renderStepAtPos(step.position, activeStepColor);

                    if (step.len > 1) { // If the step is longer than 1
                        // Find the position of the next step, accounting for wrapping
                        int nextStepOffset = *stepCount; // default if no step comes after
                        for (const auto& next : *steps) {
                            if (next.enabled && next.len > 0) {
                                int delta = (next.position - step.position + *stepCount) % *stepCount;
                                if (delta > 0 && delta < nextStepOffset) {
                                    nextStepOffset = delta;
                                }
                            }
                        }

                        int effectiveLength = std::min<int>(step.len, nextStepOffset);

                        // Render length background (excluding the first step already rendered previously)
                        for (int l = 1; l < effectiveLength; ++l) {
                            int pos = (step.position + l) % *stepCount;
                            renderStepAtPos(pos, stepLengthColor);
                        }
                    }
                }
            }
        }

        // Show selected step
        if (contextId != 0) {
            int selectedStep = view->contextVar[contextId];
            if (selectedStep < *stepCount) {
                int stepRow = selectedStep / stepPerRow;
                int stepCol = selectedStep % stepPerRow;
                int x = relativePosition.x + stepCol * stepWidth;
                int y = relativePosition.y + stepRow * stepHeight;
                draw.rect({ x + 1, y + 1 }, { stepWidth - 2, stepHeight - 3 /* should be 2 but for whatever reason it's 3, might be due to size */ }, { stepSelectedColor });
            }
        }
    }

    void renderWhichStepIsPlaying(int stepPos, bool reset = false)
    {
        int stepRow = stepPos / stepPerRow;
        int stepCol = stepPos % stepPerRow;
        int x = relativePosition.x + stepCol * stepWidth;
        int y = relativePosition.y + stepRow * stepHeight;
        // draw.filledCircle({ (int)(x + stepWidth * 0.5), (int)(y + stepHeight * 0.5) }, stepHeight * 0.25, { playingColor });
        draw.line({ x + 1, y + stepHeight - 1 }, { x + stepWidth - 2, y + stepHeight - 1 }, { reset ? background : playingColor });
    }

    void onContext(uint8_t index, float value) override
    {
        if (index == contextId) {
            renderNext();
        }
        Component::onContext(index, value);
    }
};
