#pragma once

#include "helpers/midiNote.h"
#include "host/constants.h" // TODO instead load from plugin seq
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
    int numNotes = 24;

    Color seqBeatColour = { 0x80, 0x80, 0x80 };
    Color seqBarColour = lighten(seqBeatColour, 0.4);
    Color seqColSeparatorColour = darken(seqBeatColour, 0.6);

    // juce::Colour seqRowWhiteKeyColour = juce::Colours::black.brighter(0.3);
    // juce::Colour seqRowBlackKeyColour = juce::Colours::black.brighter(0.35);
    // juce::Colour seqRowSeparatorColour = juce::Colours::black.brighter(0.40);

    Color seqRowBlackKeyColour;
    Color seqRowWhiteKeyColour;
    Color seqRowSeparatorColour;

public:
    SequencerComponent(ComponentInterface::Props props)
        : Component(props)
        , background(styles.colors.background)
        , seqRowBlackKeyColour(styles.colors.background)
        , seqRowWhiteKeyColour(lighten(styles.colors.background, 0.2))
        , seqRowSeparatorColour(lighten(styles.colors.background, 0.4))
    {
        resize();
    }

    void resize()
    {
        stepWidth = size.w / numSteps;
        stepHeight = size.h / numNotes;
    }

    void render()
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { background });

            int midiStartNote = MIDI_NOTE_C4;
            // Draw Grid with Piano Roll Styling & Note Names
            for (int i = 0; i < numNotes; ++i) {
                int y = relativePosition.y + i * stepHeight;
                int x = relativePosition.x;
                int midiNote = midiStartNote + i;
                Color color = isBlackKey(midiNote) ? seqRowBlackKeyColour : seqRowWhiteKeyColour;
                draw.filledRect({ x, y }, { size.w, stepHeight }, { color });
                draw.line({ x, y }, { x + size.w, y }, { seqRowSeparatorColour });

                // // g.setColour(midiPitch % 12 == 0 ? seqNoteColour : seqNoteLighterColour);
                // if (midiPitch % 12 == 0) {
                //     g.setColour(seqNoteColour);
                //     g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
                // } else {
                //     g.setColour(seqNoteLighterColour);
                //     g.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::plain)));
                // }
                // g.drawText(juce::MidiMessage::getMidiNoteName(midiPitch, true, true, 4), 2, y, 40, stepHeight, juce::Justification::centredLeft);
                // g.drawText(juce::MidiMessage::getMidiNoteName(midiPitch, true, true, 4), 2 + stepWidth * 16, y, 40, stepHeight, juce::Justification::centredLeft);
                // g.drawText(juce::MidiMessage::getMidiNoteName(midiPitch, true, true, 4), 2 + stepWidth * 32, y, 40, stepHeight, juce::Justification::centredLeft);
                // g.drawText(juce::MidiMessage::getMidiNoteName(midiPitch, true, true, 4), 2 + stepWidth * 48, y, 40, stepHeight, juce::Justification::centredLeft);
            }

            // Draw Beat & Bar Separations
            for (int i = 0; i <= numSteps; ++i) {
                int x = relativePosition.x + i * stepWidth;
                int y = relativePosition.y;
                Color color;
                if (i % 16 == 0)
                    color = seqBarColour; // Bar line
                else if (i % 4 == 0)
                    color = seqBeatColour; // Beat line
                else
                    color = seqColSeparatorColour;

                draw.line({ x, y }, { x, y + size.h }, { color });
            }
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        /*md - `BACKGROUND_COLOR: color` is the color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            background = draw.getColor(value);
            return true;
        }

        return false;
    }
};
