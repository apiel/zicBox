#pragma once

#include "helpers/midiNote.h"
#include "host/constants.h" // TODO instead load from plugin seq
#include "plugins/audio/stepInterface.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <string>

/*md
## Sequencer

Sequencer components to draw the sequencer interface.
*/

class SequencerComponent : public Component {
protected:
    Color background;

    int toolboxHeight = 20;
    int stepWidth = 0;
    int stepHeight = 10;
    int numSteps = MAX_STEPS; // TODO instead load from plugin seq
    int numNotes = 25;
    bool drawNoteStr = true;

    Color beatColor = { 0x80, 0x80, 0x80 };
    Color barColor = lighten(beatColor, 0.4);
    Color colSeparatorColor = darken(beatColor, 0.6);

    Color blackKeyColor;
    Color whiteKeyColor;
    Color rowSeparatorColor;

    Color textColor;
    Color text2Color;

    Color stepColor;

    Color selectedColor;

    AudioPlugin* plugin;
    std::vector<Step>* steps = NULL;

    int selectedStep = 0;
    int selectedNote = MIDI_NOTE_C5;
    int midiStartNote = MIDI_NOTE_C4;

    int parameterSelection = 0;

public:
    SequencerComponent(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == ".toggleParam") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        parameterSelection = (parameterSelection + 1) % 3;
                        renderNext();
                    }
                };
            }
            return func;
        })
        , background(styles.colors.background)
        , blackKeyColor(styles.colors.background)
        , whiteKeyColor(lighten(styles.colors.background, 0.2))
        , rowSeparatorColor(lighten(styles.colors.background, 0.4))
        , textColor(darken(styles.colors.white, 0.2))
        , text2Color(lighten(styles.colors.background, 1.5))
        , stepColor(styles.colors.primary)
        , selectedColor(styles.colors.white)
    {
        /*md md_config:Sequencer */
        nlohmann::json& config = props.config;

        /// The audio plugin sequencer.
        plugin = getPluginPtr(config, "audioPlugin", track); //eq: "audio_plugin_name"

        /// The data id to get steps from audio plugin sequencer.
        steps = (std::vector<Step>*)plugin->data(plugin->getDataId(config.value("stepsDataId", "STEPS"))); //eg: "STEPS"

        /// The number of piano notes to display on the screen.
        numNotes = config.value("numNotes", numNotes); //eg: 24

        /// The background color of the text.
        background = draw.getColor(config["bgColor"], background); //eg: "#000000"

        /// The color of the black keys.
        blackKeyColor = draw.getColor(config["blackKeyColor"], blackKeyColor); //eg: "#000000"

        /// The color of the white keys.
        whiteKeyColor = draw.getColor(config["whiteKeyColor"], whiteKeyColor); //eg: "#000000"

        /// The color of the row separators.
        rowSeparatorColor = draw.getColor(config["rowSeparatorColor"], rowSeparatorColor); //eg: "#000000"

        /// The color of the beats.
        beatColor = draw.getColor(config["beatColor"], beatColor); //eg: "#000000"

        /// The color of the bars.
        barColor = draw.getColor(config["barColor"], barColor); //eg: "#000000"

        /// The color of the column separators.
        colSeparatorColor = draw.getColor(config["colSeparatorColor"], colSeparatorColor); //eg: "#000000"

        /// The color of the C notes and parameter values.
        textColor = draw.getColor(config["textColor"], textColor); //eg: "#000000"

        /// The color of the other notes and parameter labels.
        text2Color = draw.getColor(config["text2Color"], text2Color); //eg: "#000000"

        /// The color of the steps.
        stepColor = draw.getColor(config["stepColor"], stepColor); //eg: "#000000"

        /// The color of the selected step.
        selectedColor = draw.getColor(config["selectedColor"], selectedColor); //eg: "#000000"

        /// Parameter selection. 0 = Velocity, 1 = Condition, 2 = Motion, -1 = Deactivated
        parameterSelection = config.value("parameterSelection", parameterSelection); //eg: 0

        /*md md_config_end */

        // Initialize selected note with first midi note
        if (steps != NULL && steps->size() > 0) {
            Step& step = (*steps)[0];
            selectedNote = step.note;
            midiStartNote =  range((selectedNote - numNotes / 2), MIDI_NOTE_C0, MIDI_NOTE_C9 - numNotes);
        }

        resize();
    }

    void resize() override
    {
        stepWidth = size.w / numSteps;
        stepHeight = (size.h - 20) / numNotes;
        drawNoteStr = stepHeight >= 10;
    }

    void render()
    {
        draw.filledRect(relativePosition, size, { background });

        // xStart for steps (margin left)
        int xStart = drawNoteStr ? size.w - stepWidth * numSteps : 0;
        if (xStart < 16) {
            xStart = 0;
        }

        // Draw Grid with Piano Roll Styling & Note Names
        for (int i = 0; i < numNotes; ++i) {
            int y = relativePosition.y + i * stepHeight;
            int x = relativePosition.x;
            int midiNote = midiStartNote + numNotes - i - 1;
            Color color = isBlackKey(midiNote) ? blackKeyColor : whiteKeyColor;
            draw.filledRect({ x, y }, { size.w, stepHeight }, { color });
            draw.line({ x, y }, { x + size.w, y }, { rowSeparatorColor });

            if (xStart) {
                if (midiNote % 12 == 0) {
                    draw.text({ x, y + 1 }, MIDI_NOTES_STR[midiNote], 8, { textColor });
                } else if (!isBlackKey(midiNote)) {
                    draw.text({ x, y + 1 }, MIDI_NOTES_STR[midiNote], 8, { text2Color });
                } else {
                    draw.text({ x, y + 1 }, " #", 8, { text2Color });
                }
            }
        }

        // Draw Beat & Bar Separations
        for (int i = 0; i <= numSteps; ++i) {
            int x = xStart + relativePosition.x + i * stepWidth;
            int y = relativePosition.y;
            Color color;
            if (i % 16 == 0)
                color = barColor; // Bar line
            else if (i % 4 == 0)
                color = beatColor; // Beat line
            else
                color = colSeparatorColor;

            draw.line({ x, y }, { x, y + size.h - toolboxHeight }, { color });
        }

        if (steps != NULL) {
            // Draw MIDI Notes
            for (const auto& step : *steps) {
                if (step.note >= midiStartNote && step.note < midiStartNote + numNotes) {
                    int x = xStart + step.position * stepWidth + 1;
                    int y = (numNotes - (step.note - midiStartNote) - 1) * stepHeight;
                    int width = step.len * stepWidth - 2;
                    int height = stepHeight;

                    draw.filledRect({ x, y }, { width, height }, { stepColor });

                    // if (selectedNote == &step) {
                    //     g.setColour(juce::Colours::white);
                    //     g.drawRect(x, y, width, height);
                    // }
                }
            }
        }

        // Draw Selection
        int x = xStart + selectedStep * stepWidth + 1;
        int y = (numNotes - (selectedNote - midiStartNote) - 1) * stepHeight;
        // draw.rect({ x, y }, { stepWidth - 1, stepHeight - 1 }, { selectedColor });
        draw.filledCircle({ x + stepWidth / 2, y + stepHeight / 2 }, 3, { selectedColor });

        // Toolbox
        y = size.h - toolboxHeight + 1;
        draw.text({ relativePosition.x, y }, "Stp Note Len Vel. Cond. Motion", 8, { text2Color });
        y += 8;
        draw.textRight({ relativePosition.x + 24, y }, std::to_string(selectedStep + 1), 8, { textColor });
        draw.text({ relativePosition.x + 32, y }, MIDI_NOTES_STR[selectedNote], 8, { stepColor });
        draw.textRight({ relativePosition.x + 96, y }, "1", 8, { textColor });
        draw.textRight({ relativePosition.x + 136, y }, "100%", 8, { textColor });
        draw.text({ relativePosition.x + 144, y }, "---", 8, { textColor });
        draw.text({ relativePosition.x + 192, y }, "---", 8, { textColor });
        y += 8;
        if (parameterSelection == 0) {
            draw.line({ relativePosition.x + 104, y }, { relativePosition.x + 136, y }, { stepColor });
        } else if (parameterSelection == 1) {
            draw.line({ relativePosition.x + 144, y }, { relativePosition.x + 176, y }, { stepColor });
        } else if (parameterSelection == 2) {
            draw.line({ relativePosition.x + 192, y }, { relativePosition.x + 232, y }, { stepColor });
        }
    }

    void onEncoder(int id, int8_t direction) override
    {
        if (id == 0) {
            direction = direction > 0 ? 1 : -1;
            selectedNote = range((selectedNote + direction), MIDI_NOTE_C0, MIDI_NOTE_C9);
            if (selectedNote < midiStartNote) {
                midiStartNote = selectedNote;
            } else if (selectedNote >= midiStartNote + numNotes) {
                midiStartNote = selectedNote - numNotes + 1;
            }
            renderNext();
        } else if (id == 1) {
            direction = direction > 0 ? 1 : -1;
            selectedStep = range((selectedStep + direction), 0, numSteps - 1);
            renderNext();
        }
    }
};
