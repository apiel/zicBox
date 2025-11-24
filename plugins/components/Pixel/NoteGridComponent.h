/** Description:
This code defines a crucial user interface element called the `NoteGridComponent`. It functions as a flexible, configurable musical controller, transforming a simple grid of buttons into a melodic instrument.

**Purpose and Functionality:**
The primary goal of this component is to generate musical messages (MIDI notes) based on user interaction with a matrix of cells (buttons). It allows the user to play notes constrained to a specific musical context, ensuring that the notes played sound harmonically correct.

**Configuration:**
The grid is highly customizable. Developers or users can define the size of the grid (number of rows and columns), the starting pitch range (octave), and the current musical pattern (scale, such as Major, Minor, or Pentatonic). It also manages all visual aspects, defining colors for the background, individual cells (including a separate color for notes that would be sharp/black keys), borders, and text labels.

**How It Works:**
When initialized, the component first determines the total number of keys. It then calculates and assigns a specific MIDI note to every cell based on the selected musical scale's intervals and the current octave setting. This ensures the grid steps sequentially through the chosen scale.

**User Interaction:**
1.  **Grid Press:** Pressing a cell on the grid triggers an immediate "Note On" message to the connected audio plugin, generating sound. Releasing the cell sends a "Note Off" message.
2.  **Encoders (Dials):** The component listens to dedicated rotary encoders (dials). One dial is configured to cycle through the available musical scales, while another changes the overall octave range. Adjusting these encoders automatically recalculates the note assignments for the entire grid, instantly updating the instrument's musical layout.
3.  **Rendering:** The component handles its own display, showing the current state, including the names of the notes in each cell and the currently selected scale and octave at the bottom.

sha: 068a6ae547386934b1419f5800ccd6f18475431b5ff331326059309d5df3a644 
*/
#pragma once

#include "helpers/midiNote.h"
#include "plugins/audio/stepInterface.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <array>
#include <string>
#include <vector>

/*md
## Keyboard Grid Component

Configurable rows × cols button matrix that maps to notes in a given scale and octave.

- Encoder 0: change scale
- Encoder 1: change octave
- Key press: noteOn
- Key release: noteOff
*/

class NoteGridComponent : public Component {
protected:
    AudioPlugin* plugin;

    int numCols = 4;
    int numRows = 3;
    int octave = 4;
    int scaleIndex = 0;
    int encScale = 0;
    int encOctave = 1;

    bool inverted = false;

    Color background;
    Color cellColor;
    Color cellSharpColor;
    Color borderColor;
    Color textColor;
    void* font = nullptr;

    std::vector<std::vector<int>> scales = {
        { 0, 2, 4, 5, 7, 9, 11 }, // Major
        { 0, 2, 3, 5, 7, 8, 10 }, // Minor
        { 0, 2, 4, 7, 9 }, // Major Pentatonic
        { 0, 3, 5, 7, 10 }, // Minor Pentatonic
    };
    std::vector<std::string> scaleNames = { "Major", "Minor", "MajPent", "MinPent" };

    std::vector<uint8_t> notes;

    void updateNotes()
    {
        notes.clear();
        const auto& scale = scales[scaleIndex];
        int baseNote = octave * 12;
        int totalKeys = numCols * numRows;

        if (inverted) {
            for (int i = 0; i < totalKeys; i++) {
                int degree = i % scale.size();
                int octaveOffset = i / scale.size();
                uint8_t note = CLAMP(baseNote + scale[degree] + octaveOffset * 12,
                    MIDI_NOTE_C0, MIDI_NOTE_C9);
                notes.push_back(note);
            }
        } else {
            for (int r = numRows - 1; r >= 0; r--) {
                for (int c = 0; c < numCols; c++) {
                    int i = r * numCols + c;
                    int degree = i % scale.size();
                    int octaveOffset = i / scale.size();
                    uint8_t note = CLAMP(baseNote + scale[degree] + octaveOffset * 12,
                        MIDI_NOTE_C0, MIDI_NOTE_C9);
                    notes.push_back(note);
                }
            }
        }
    }

public:
    NoteGridComponent(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action.rfind(".key:", 0) == 0) {
                int keyIndex = atoi(action.substr(5).c_str());
                if (keyIndex >= 0) {
                    bool record = true;
                    func = [this, keyIndex, record](KeypadLayout::KeyMap& keymap) {
                        if (keyIndex >= notes.size())
                            return;
                        if (KeypadLayout::isPressed(keymap)) {
                            plugin->noteOn(notes[keyIndex], 1.0f, (void*)&record);
                        } else if (KeypadLayout::isReleased(keymap)) {
                            plugin->noteOff(notes[keyIndex], 0.0f, (void*)&record);
                        }
                    };
                }
            }
            return func;
        })
        , background(styles.colors.background)
        , cellColor(lighten(styles.colors.background, 0.5))
        , cellSharpColor(lighten(styles.colors.background, 0.2))
        , borderColor(lighten(styles.colors.background, 1.0))
        , textColor(alpha(styles.colors.text, 0.4))
    {
        font = draw.getFont("PoppinsLight_8");

        nlohmann::json& config = props.config;
        plugin = getPluginPtr(config, "audioPlugin", track);

        numCols = config.value("cols", numCols);
        numRows = config.value("rows", numRows);
        octave = config.value("octave", octave);
        scaleIndex = config.value("scaleIndex", scaleIndex);
        encScale = config.value("encScale", encScale);
        encOctave = config.value("encOctave", encOctave);
        inverted = config.value("inverted", inverted);

        background = draw.getColor(config["bgColor"], background);
        cellColor = draw.getColor(config["cellColor"], cellColor);
        cellSharpColor = draw.getColor(config["cellSharpColor"], cellSharpColor);
        borderColor = draw.getColor(config["borderColor"], borderColor);
        textColor = draw.getColor(config["textColor"], textColor);

        updateNotes();
    }

    void onEncoder(int8_t id, int8_t direction) override
    {
        direction = direction > 0 ? 1 : -1;
        if (id == encScale) {
            scaleIndex = (scaleIndex + direction + scales.size()) % scales.size();
            updateNotes();
            renderNext();
        } else if (id == encOctave) {
            octave = CLAMP(octave + direction, 0, 9);
            updateNotes();
            renderNext();
        }
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { background });

        int gridH = size.h - 10;
        int cellW = size.w / numCols;
        int cellH = gridH / numRows;

        for (int r = 0; r < numRows; r++) {
            for (int c = 0; c < numCols; c++) {
                int idx = r * numCols + c;
                if (idx >= (int)notes.size())
                    continue;

                int x = relativePosition.x + c * cellW;
                int y = relativePosition.y + r * cellH;

                draw.filledRect({ x, y }, { cellW, cellH }, { isBlackKey(notes[idx]) ? cellSharpColor : cellColor });
                draw.rect({ x, y }, { cellW, cellH }, { borderColor });
                draw.textCentered({ x + cellW / 2, y + cellH / 2 }, MIDI_NOTES_STR[notes[idx]], 8, { textColor, .font = font });
            }
        }

        draw.text({ relativePosition.x + 2, relativePosition.y + size.h - 10 },
            scaleNames[scaleIndex], 8, { textColor, .font = font });
        draw.text({ (int)(relativePosition.x + size.w * 0.5), relativePosition.y + size.h - 10 },
            "Oct:" + std::to_string(octave), 8, { textColor, .font = font });
    }
};
