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

    Color noteColor;
    Color note2Color;

    Color stepColor;

    AudioPlugin* plugin;
    std::vector<Step>* steps = NULL;

public:
    SequencerComponent(ComponentInterface::Props props)
        : Component(props)
        , background(styles.colors.background)
        , blackKeyColor(styles.colors.background)
        , whiteKeyColor(lighten(styles.colors.background, 0.2))
        , rowSeparatorColor(lighten(styles.colors.background, 0.4))
        , noteColor(darken(styles.colors.white, 0.2))
        , note2Color(lighten(styles.colors.background, 1.5))
        , stepColor(styles.colors.primary)
    {
        /*md md_config:Sequencer */
        nlohmann::json config = props.config;

        // TODO make a getter function
        /// The audio plugin sequencer. */
        /*md   audioPlugin="audio_plugin_name" */
        if (!config.contains("audioPlugin")) {
            logWarn("Sequencer component is missing audioPlugin parameter.");
            return;
        }
        plugin = &getPlugin(config["audioPlugin"].get<std::string>().c_str(), track);

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

        /// The color of the C notes.
        noteColor = draw.getColor(config["noteColor"], noteColor); //eg: "#000000"

        /// The color of the other notes.
        note2Color = draw.getColor(config["note2Color"], note2Color); //eg: "#000000"

        /// The color of the steps.
        stepColor = draw.getColor(config["stepColor"], stepColor); //eg: "#000000"

        /*md md_config_end */

        resize();
    }

    void resize() override
    {
        stepWidth = size.w / numSteps;
        stepHeight = size.h / numNotes;
        drawNoteStr = stepHeight >= 10;
    }

    void render()
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { background });

            // xStart for steps (margin left)
            int xStart = drawNoteStr ? size.w - stepWidth * numSteps : 0;
            if (xStart < 16) {
                xStart = 0;
            }

            int midiStartNote = MIDI_NOTE_C4;
            // Draw Grid with Piano Roll Styling & Note Names
            for (int i = 0; i < numNotes; ++i) {
                int y = relativePosition.y + i * stepHeight;
                int x = relativePosition.x;
                int midiNote = midiStartNote + numNotes - i -1;
                Color color = isBlackKey(midiNote) ? blackKeyColor : whiteKeyColor;
                draw.filledRect({ x, y }, { size.w, stepHeight }, { color });
                draw.line({ x, y }, { x + size.w, y }, { rowSeparatorColor });

                if (xStart) {
                    if (midiNote % 12 == 0) {
                        draw.text({ x, y + 1 }, MIDI_NOTES_STR[midiNote], 8, { noteColor });
                    } else if (!isBlackKey(midiNote)) {
                        draw.text({ x, y + 1 }, MIDI_NOTES_STR[midiNote], 8, { note2Color });
                    } else {
                        draw.text({ x, y + 1 }, " #", 8, { note2Color });
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

                draw.line({ x, y }, { x, y + size.h }, { color });
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
        }
    }
};
