/** Description:
This code defines a specialized software component named `StepEditSampleComponent`. Its purpose is to act as a user interface element for graphically editing a single, specific "step" within an audio sequencer that plays sampled sounds.

### Core Functionality

This component allows a user to configure four key parameters related to how an audio sample is played at a specific point in a sequence:

1.  **Enable/Disable:** It can toggle the step on or off.
2.  **Sample Selection:** It links to the audio engine to select which audio file (sample) is used.
3.  **Velocity (Volume):** It controls the volume or intensity of the triggered sound.
4.  **Start/End Points:** It defines the exact portion of the sample file to be played (e.g., setting the start to 50% means the sound only begins halfway through the file).

### User Interaction and Display

The component handles input from physical controls, such as rotary encoders (knobs). Each of the four expected encoders is mapped to a specific function: controlling velocity, moving to the next or previous sample file, and adjusting the sample’s start or end points.

Visually, the component provides immediate feedback:
*   It displays the name of the currently selected audio file.
*   It shows a small bar representing the velocity (volume) level.
*   It displays numerical values for the sample start and end points as percentages.
*   It integrates with the master sequencer, dynamically changing its color to highlight when the associated step is currently being played, ensuring the user always knows what is active.

In summary, this class provides the visual layer and input processing necessary for a user to precisely modify a single sample trigger event within an audio production environment.

sha: 3026b1c6f5d4b0bfced1503b56336dd4d05079be154a0560081d517ce257acd6 
*/
#pragma once

#include "helpers/format.h"
#include "helpers/midiNote.h"
#include "plugins/audio/SampleStep.h"
#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <filesystem>
#include <stdexcept>

/*md
## StepEditSample

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/stepEditSample.png" />

StepEditSample component is used to edit a step value for samples.
*/

class StepEditSampleComponent : public Component {
protected:
    AudioPlugin* plugin = NULL;
    SampleStep* step;
    uint8_t* stepCounter = NULL;

    uint8_t nextFileDataId = 0;
    uint8_t prevFileDataId = 0;
    uint8_t playStepDataId = 0;

    bool notePlaying = false;
    bool* seqPlayingPtr = NULL;
    bool seqPlaying = false;

    Color bgColor;
    Color selection;
    Color fileColor;
    Color note2Color;
    Color text;
    Color text2;
    Color barBackground;
    Color bar;
    Color textMotion1;
    Color textMotion2;
    Color playingColor;
    // Color playingBgColor;

    uint8_t stepIndex = -1;

    uint8_t encoders[4] = { 0, 1, 2, 3 };

public:
    StepEditSampleComponent(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == ".toggle") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        step->enabled = !step->enabled;
                        renderNext();
                    }
                };
            }
            if (action == ".play") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        plugin->data(playStepDataId, &stepIndex);
                    }
                };
            }
            return func;
        })
        , bgColor(styles.colors.background)
        , selection(lighten(styles.colors.background, 0.5))
        , fileColor(styles.colors.primary)
        , note2Color(styles.colors.white)
        , text(styles.colors.text)
        , text2(darken(styles.colors.text, 0.3))
        , barBackground(darken(styles.colors.tertiary, 0.5))
        , bar(styles.colors.tertiary)
        , textMotion1(styles.colors.secondary)
        , textMotion2(styles.colors.quaternary)
        , playingColor(styles.colors.tertiary)
    {
        jobRendering = [this](unsigned long now) {
            if (seqPlayingPtr != NULL && *seqPlayingPtr != seqPlaying) {
                seqPlaying = *seqPlayingPtr;
                renderNext();
            }
            if (stepCounter != NULL && seqPlaying) {
                if (stepIndex != *stepCounter) {
                    if (notePlaying) {
                        notePlaying = false;
                        renderNext();
                    }
                } else if (!notePlaying) {
                    notePlaying = true;
                    renderNext();
                }
            }
        };

        /*md md_config:StepEditSample */
        nlohmann::json& config = props.config;

        /// The audio plugin to get control on.
        plugin = getPluginPtr(config, "audioPlugin", track); //eq: "audio_plugin_name"

        /// Index of the step.
        stepIndex = getConfig<uint8_t>(config, "stepIndex"); //eg: 0

        step = (SampleStep*)plugin->data(plugin->getDataId("GET_STEP"), &stepIndex);
        nextFileDataId = plugin->getDataId("NEXT_FILE");
        prevFileDataId = plugin->getDataId("PREVIOUS_FILE");
        playStepDataId = plugin->getDataId("PLAY_STEP");

        /// The background color of the text.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        /// The color of the text
        text = draw.getColor(config["textColor"], text); //eg: "#ffffff"

        /// The color of the actual playing step.
        playingColor = draw.getColor(config["playingColor"], playingColor); //eg: "#ffffff"

        /// The color of the selected step.
        selection = draw.getColor(config["selectedColor"], selection); //eg: "#ffffff"

        /*md md_config_end */
    }

    int renderPct(int x, int y, float pct)
    {
        x = draw.text({ x + 2, y }, std::to_string((int)(pct)), 8, { text });
        draw.text({ x - 4, y }, ".", 8, { text });
        int decimal = (pct - (int)pct) * 10;
        x = draw.text({ x + 2, y }, std::to_string(decimal), 8, { text2 });
        x = draw.text({ x, y + 4 }, "%", 4, { text2 });

        return x;
    }

    void renderStartEnd(int y, Color bg)
    {
        int x = relativePosition.x + size.w - 84;
        draw.filledRect({ x, y }, { 84, 8 }, { bg });

        renderPct(x + 2, y, step->fStart);
        renderPct(x + 42, y, step->fEnd);
    }

    void render() override
    {
        if (step) {
            Color bg = selection;
            draw.filledRect(relativePosition, size, { bg });

            int y = relativePosition.y;
            int x = relativePosition.x + 1;

            x = relativePosition.x + 2;
            draw.filledRect({ x, y + 2 }, { 20, 4 }, { barBackground });
            draw.filledRect({ x, y + 2 }, { (int)(20 * step->velocity), 4 }, { bar });

            x = relativePosition.x + 25;
            if (step->enabled) {
                draw.text({ x, y }, std::filesystem::path(step->filename).filename(), 8, { fileColor });

                // if (step->fStart != 0 || step->fEnd != 100) {
                renderStartEnd(y, bg);
                // }
            } else {
                draw.text({ x, y }, "---", 8, { text2 });
            }
        }
    }

    void onEncoder(int8_t id, int8_t direction) override
    {
        if (id == encoders[0]) {
            step->setVelocity(step->velocity + direction * 0.05);
            renderNext();
        } else if (id == encoders[1]) {
            if (direction > 0) {
                plugin->data(nextFileDataId, &stepIndex);
            } else {
                plugin->data(prevFileDataId, &stepIndex);
            }
            renderNext();
        } else if (id == encoders[2]) {
            step->setStart(step->fStart + direction * 0.1);
            renderNext();
        } else if (id == encoders[3]) {
            step->setEnd(step->fEnd + direction * 0.1);
            renderNext();
        }
    }

    bool onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        return keypadLayout.onKey(id, key, state, now);
    }
};
