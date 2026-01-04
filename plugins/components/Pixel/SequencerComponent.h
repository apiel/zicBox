/** Description:
This file serves as the technical blueprint for a **Sequencer Component**, which is a graphical user interface element designed to display and edit musical patterns. This component provides a visual interface similar to a classic "piano roll."

**Core Function:**
The main job of this component is to visualize and manage the steps of a musical sequence, linking the visual display to the actual musical data stored within an associated **Audio Plugin**. It determines how to draw the grid, where existing musical notes (or "steps") are located, and how to highlight the user's current selection.

**Structure and Display:**
The sequencer drawing function (`render`) creates a grid combining musical notes (vertical axis, like a piano keyboard) and time steps (horizontal axis). It uses extensive color definitions to differentiate visual elements like black and white keys, beats, bars, and the selected step. A dedicated "toolbox" area at the bottom provides detailed information about the currently focused note (such as its velocity or length).

**Interaction:**
The component is highly interactive. It defines specific responses to user input:
1.  **Navigation:** Allows users to move the selection cursor up/down (changing the note) or left/right (changing the step position).
2.  **Editing:** Users can instantly toggle a step on or off, or delete an existing step.
3.  **Parameter Control:** Specialized functions handle input from rotary knobs or encoders to adjust detailed musical parameters of the selected step, such as volume (velocity), duration, or any associated musical 'motion' effect.

In essence, this code defines a self-contained visual editor that translates user actions directly into changes in the song's fundamental musical data.

sha: c468b7272cf8fc3d729a10671f0adc0cd140d3e530e23b26e0c26f77995a7635 
*/
#pragma once

#include "helpers/midiNote.h"
#include "host/constants.h" // TODO instead load from plugin seq
#include "plugins/audio/stepInterface.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <string>

/*md
## Sequencer

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/sequencer.png" />

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/sequencer2.png" />

Sequencer components to draw the sequencer interface.
*/

class SequencerComponent : public Component {
protected:
    int toolboxHeight = 20;
    int stepWidth = 0;
    int stepHeight = 10;
    int numSteps = DEFAULT_MAX_STEPS; // TODO instead load from plugin seq
    int maxStepLen = DEFAULT_MAX_STEPS;
    int numNotes = 25;
    bool drawNoteStr = true;

    Color background;
    Color beatColor;
    Color barColor;
    Color colSeparatorColor;
    Color blackKeyColor;
    Color whiteKeyColor;
    Color rowSeparatorColor;
    Color textColor;
    Color text2Color;
    Color stepColor;
    Color selectedColor;
    Color textMotion1;
    Color textMotion2;

    AudioPlugin* plugin;
    std::vector<Step>* steps = NULL;

    int selectedStep = 0;
    int selectedNote = MIDI_NOTE_C5;
    int midiStartNote = MIDI_NOTE_C4;

    int parameterSelection = 0;
    bool shift = false;

    void* font = NULL;

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
            if (action == ".shift") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isPressed(keymap)) {
                        shift = true;
                    } else if (KeypadLayout::isReleased(keymap)) {
                        shift = false;
                    }
                };
            }
            if (action == ".noteOn") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    bool record = false;
                    if (KeypadLayout::isPressed(keymap)) {
                        Step* step = getSelectedStep();
                        float velo = step ? step->velocity : 1.0f;
                        plugin->noteOn(selectedNote, velo, &record);
                    } else if (KeypadLayout::isReleased(keymap)) {
                        plugin->noteOff(selectedNote, 0.0f, &record);
                    }
                };
            }
            if (action == ".selectNoteUp") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        selectNote(1);
                    }
                };
            }
            if (action == ".selectNoteDown") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        selectNote(-1);
                    }
                };
            }
            if (action == ".selectStepRight") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        selectStep(1);
                    }
                };
            }
            if (action == ".selectStepLeft") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        selectStep(-1);
                    }
                };
            }
            if (action == ".toggleStep") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        toggleStep();
                    }
                };
            }
            if (action == ".deleteStep") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        Step* step = getSelectedStep();
                        if (step != nullptr) {
                            deleteStep(step);
                        }
                    }
                };
            }
            return func;
        })
        , beatColor(lighten(styles.colors.background, 1.0))
        , barColor(lighten(styles.colors.background, 2.0))
        // , colSeparatorColor(lighten(styles.colors.background, 0.7))
        , colSeparatorColor(darken(styles.colors.background, 0.3))
        , background(styles.colors.background)
        , blackKeyColor(styles.colors.background)
        , whiteKeyColor(lighten(styles.colors.background, 0.2))
        , rowSeparatorColor(lighten(styles.colors.background, 0.4))
        , textColor(darken(styles.colors.white, 0.2))
        , text2Color(lighten(styles.colors.background, 1.5))
        , stepColor(styles.colors.primary)
        , selectedColor(styles.colors.white)
        , textMotion1(styles.colors.secondary)
        , textMotion2(styles.colors.quaternary)
    {
        font = draw.getFont("PoppinsLight_8");

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

        /// The color of the motion 1.
        textMotion1 = draw.getColor(config["textMotion1"], textMotion1); //eg: "#000000"

        /// The color of the motion 2.
        textMotion2 = draw.getColor(config["textMotion2"], textMotion2); //eg: "#000000"

        /// Parameter selection. 0 = Velocity, 1 = Condition, 2 = Motion, -1 = Deactivated
        parameterSelection = config.value("parameterSelection", parameterSelection); //eg: 0

        /// Max step length
        maxStepLen = config.value("maxStepLen", maxStepLen); //eg: 1

        /*md md_config_end */

        // Initialize selected note with first midi note
        if (steps != NULL && steps->size() > 0) {
            Step& step = (*steps)[0];
            selectedNote = step.note;
            midiStartNote = CLAMP((selectedNote - numNotes / 2), MIDI_NOTE_C0, MIDI_NOTE_C9 - numNotes);
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
                    draw.text({ x, y + 1 }, MIDI_NOTES_STR[midiNote], 8, { textColor, .font = font });
                } else if (!isBlackKey(midiNote)) {
                    draw.text({ x, y + 1 }, MIDI_NOTES_STR[midiNote], 8, { text2Color, .font = font });
                } else {
                    draw.text({ x, y + 1 }, " #", 8, { text2Color, .font = font });
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
                if (step.len && step.enabled && step.note >= midiStartNote && step.note < midiStartNote + numNotes) {
                    int x = xStart + step.position * stepWidth + 1;
                    int y = (numNotes - (step.note - midiStartNote) - 1) * stepHeight;

                    if (step.position + step.len > numSteps) {
                        int width = (numSteps - step.position) * stepWidth - 2;
                        draw.filledRect({ x, y }, { width, stepHeight }, { stepColor });
                        width = (step.len - (numSteps - step.position)) * stepWidth - 2;
                        draw.filledRect({ xStart + 1, y }, { width, stepHeight }, { stepColor });
                    } else {
                        int width = step.len * stepWidth - 2;
                        draw.filledRect({ x, y }, { width, stepHeight }, { stepColor });
                    }
                }
            }
        }

        // Draw Selection
        int x = xStart + selectedStep * stepWidth + 1;
        int y = (numNotes - (selectedNote - midiStartNote) - 1) * stepHeight;
        // draw.rect({ x, y }, { stepWidth - 1, stepHeight - 1 }, { selectedColor });
        draw.filledCircle({ x + stepWidth / 2, y + stepHeight / 2 }, 3, { selectedColor });

        // Toolbox
        Step* step = getSelectedStep();
        y = size.h - toolboxHeight + 1;
        // "Step Note Len Vel. Cond. Motion"
        draw.text({ relativePosition.x + 2, y }, "Step", 8, { text2Color, .font = font });
        draw.text({ relativePosition.x + 32, y }, "Note", 8, { text2Color, .font = font });
        draw.text({ relativePosition.x + 64, y }, "Len", 8, { text2Color, .font = font });
        draw.text({ relativePosition.x + 86, y }, "Vel.", 8, { text2Color, .font = font });
        draw.text({ relativePosition.x + 120, y }, "Cond.", 8, { text2Color, .font = font });
        draw.text({ relativePosition.x + 160, y }, "Motion", 8, { text2Color, .font = font });
        y += 8;
        draw.textRight({ relativePosition.x + 22, y }, std::to_string(step ? step->position + 1 : selectedStep + 1), 8, { textColor, .font = font });
        draw.text({ relativePosition.x + 32, y }, MIDI_NOTES_STR[selectedNote], 8, { stepColor, .font = font });
        draw.textRight({ relativePosition.x + 78, y }, std::to_string(step ? step->len : 0), 8, { textColor, .font = font });
        draw.textRight({ relativePosition.x + 110, y }, step ? std::to_string((int)(step->velocity * 100)) + "%" : "---", 8, { textColor, .font = font });
        draw.text({ relativePosition.x + 120, y }, step ? stepConditions[step->condition].name : "---", 8, { textColor, .font = font });
        // draw.text({ relativePosition.x + 192, y }, step ? stepMotions[step->motion].name : "---", 8, { textColor, .font = font });
        if (step == NULL || step->motion == 0) {
            draw.text({ relativePosition.x + 160, y }, "---", 8, { textColor, .font = font });
        } else {
            renderStepMotion({ relativePosition.x + 160, y }, stepMotions[step->motion].name);
        }
        y += 8;
        if (parameterSelection == 0) {
            draw.line({ relativePosition.x + 86, y }, { relativePosition.x + 110, y }, { stepColor });
        } else if (parameterSelection == 1) {
            draw.line({ relativePosition.x + 120, y }, { relativePosition.x + 150, y }, { stepColor });
        } else if (parameterSelection == 2) {
            draw.line({ relativePosition.x + 160, y }, { relativePosition.x + 190, y }, { stepColor });
        }
    }

    void renderStepMotion(Point pos, std::string motionSteps)
    {
        int x = pos.x;
        char* motionStep = strtok((char*)motionSteps.c_str(), ",");
        for (int i = 0; motionStep != NULL; i++) {
            x = draw.text({ x, pos.y }, motionStep, 8, { i % 2 == 0 ? textMotion1 : textMotion2, .font = font });
            motionStep = strtok(NULL, ",");
        }
    }

    Step* getSelectedStep()
    {
        for (auto& step : *steps) {
            if (step.note == selectedNote && (selectedStep == step.position || (selectedStep >= step.position && selectedStep < step.position + step.len))) {
                return &step;
            }
        }
        return nullptr;
    }

    void selectNote(int8_t direction)
    {
        selectedNote = CLAMP((selectedNote + direction), MIDI_NOTE_C0, MIDI_NOTE_C9);
        if (selectedNote < midiStartNote) {
            midiStartNote = selectedNote;
        } else if (selectedNote >= midiStartNote + numNotes) {
            midiStartNote = selectedNote - numNotes + 1;
        }
        renderNext();
    }

    void selectStep(int8_t direction)
    {
        selectedStep = CLAMP((selectedStep + direction), 0, numSteps - 1);
        renderNext();
    }

    void deleteStep(Step* step)
    {
        for (int i = 0; i < steps->size(); i++) {
            if (step == &steps->at(i)) {
                steps->erase(steps->begin() + i);
                break;
            }
        }
        renderNext();
    }

    void toggleStep()
    {
        Step* step = getSelectedStep();
        if (step != nullptr) {
            step->enabled = !step->enabled;
        } else {
            // Create a step and push it to the end
            Step newStep;
            newStep.note = selectedNote;
            newStep.position = selectedStep;
            newStep.enabled = true;
            newStep.len = 1;
            steps->push_back(newStep);
        }
        renderNext();
    }

    void onEncoder(int8_t id, int8_t direction) override
    {
        direction = direction > 0 ? 1 : -1;
        if (id == 0) {
            selectNote(direction);
        } else if (id == 1) {
            selectStep(direction);
        } else if (id == 2) {
            Step* step = getSelectedStep();
            if (step != nullptr) {
                step->enabled = true;
                step->len = CLAMP((step->len + direction), 0, maxStepLen);
                // Use shift to delete the step
                if (shift && step->len == 0) {
                    deleteStep(step);
                    return;
                }
                // TODO check if len doesn't conflict with another step
            } else if (direction > 0) {
                // Create a step and push it to the end
                Step newStep;
                newStep.note = selectedNote;
                newStep.position = selectedStep;
                newStep.enabled = true;
                steps->push_back(newStep);
            }
            renderNext();
        } else if (id == 3) {
            Step* step = getSelectedStep();
            if (step != nullptr) {
                if (parameterSelection == 0) {
                    step->setVelocity(step->velocity + direction * 0.01);
                } else if (parameterSelection == 1) {
                    step->setCondition(step->condition + direction);
                } else if (parameterSelection == 2) {
                    step->setMotion(step->motion + direction);
                }
                renderNext();
            }
        }
    }
};
