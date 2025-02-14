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

    Color seqRowBlackKeyColour;
    Color seqRowWhiteKeyColour;
    Color seqRowSeparatorColour;

    Color seqNoteColour;
    Color seqNote2Colour;

public:
    SequencerComponent(ComponentInterface::Props props)
        : Component(props)
        , background(styles.colors.background)
        , seqRowBlackKeyColour(styles.colors.background)
        , seqRowWhiteKeyColour(lighten(styles.colors.background, 0.2))
        , seqRowSeparatorColour(lighten(styles.colors.background, 0.4))
        , seqNoteColour(darken(styles.colors.white, 0.3))
        , seqNote2Colour(lighten(styles.colors.background, 1.5))
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

                if (midiNote % 12 == 0) {
                    draw.text({ x, y + 1 }, MIDI_NOTES_STR[midiNote], 8, { seqNoteColour });
                } else if (!isBlackKey(midiNote)) {
                    draw.text({ x, y + 1 }, MIDI_NOTES_STR[midiNote], 8, { seqNote2Colour });
                } else {
                    draw.text({ x, y + 1 }, " #", 8, { seqNote2Colour });
                }
            }

            // Draw Beat & Bar Separations
            int xStart = size.w - stepWidth * numSteps;
            for (int i = 0; i <= numSteps; ++i) {
                int x = xStart + relativePosition.x + i * stepWidth;
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
