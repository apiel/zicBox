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
    int octave = 4; // default octave
    int scaleIndex = 0;
    int encScale = 0;
    int encOctave = 1;

    Color background;
    Color textColor;
    void* font = nullptr;

    bool inverted = false;

    // Some example scales (intervals in semitones)
    std::vector<std::vector<int>> scales = {
        { 0, 2, 4, 5, 7, 9, 11 }, // Major
        { 0, 2, 3, 5, 7, 8, 10 }, // Minor
        { 0, 2, 4, 7, 9 }, // Major Pentatonic
        { 0, 3, 5, 7, 10 }, // Minor Pentatonic
    };
    std::vector<std::string> scaleNames = { "Major", "Minor", "MajPent", "MinPent" };

    std::vector<uint8_t> notes; // dynamic grid notes

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
                uint8_t note = CLAMP(baseNote + scale[degree] + octaveOffset * 12, MIDI_NOTE_C0, MIDI_NOTE_C9);
                notes.push_back(note);
            }
        } else {
           for (int r = numRows - 1; r >= 0; r--) { // bottom row first
                for (int c = 0; c < numCols; c++) {
                    int i = r * numCols + c; // grid index
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
        , textColor(styles.colors.white)
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

        updateNotes();
    }

    void onEncoder(int id, int8_t direction) override
    {
        direction = direction > 0 ? 1 : -1;
        if (id == encScale) { // scale select
            scaleIndex = (scaleIndex + direction + scales.size()) % scales.size();
            updateNotes();
            renderNext();
        } else if (id == encOctave) { // octave shift
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

                draw.rect({ x, y }, { cellW, cellH }, { textColor });
                draw.textCentered({ x + cellW / 2, y + cellH / 2 }, MIDI_NOTES_STR[notes[idx]], 8, { textColor, .font = font });
            }
        }

        // Draw scale/octave info at bottom
        draw.text({ relativePosition.x + 2, relativePosition.y + size.h - 10 }, scaleNames[scaleIndex], 8, { textColor, .font = font });
        draw.text({ (int)(relativePosition.x + size.w * 0.5), relativePosition.y + size.h - 10 }, "Oct:" + std::to_string(octave), 8, { textColor, .font = font });
    }
};
