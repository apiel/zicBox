/** Description:
### StepEdit2 Component Overview

This file defines the structure for a specialized user interface element called the `StepEdit2Component`. Its primary function is to provide a dedicated, interactive screen display for adjusting the detailed parameters of a single musical note or event, known as a "step," within a sequencing system.

#### Core Functionality

The component acts as a graphical window onto one specific step in a musical pattern. It handles the display and modification of all crucial step data:

1.  **Note Information:** The pitch of the note (like C4, F#5).
2.  **Velocity:** The volume or intensity of the note strike.
3.  **Length:** How long the note is held.
4.  **Playback Rules:** Advanced settings like *conditions* (when the step should play) and *motion* (complex behavioral patterns applied to the step).

#### User Interaction

Interaction is primarily handled through specialized hardware controls:

*   **Encoders (Knobs):** The component utilizes three designated rotary encoders. In its standard mode, these knobs adjust the note’s velocity, pitch, and length. If a "shift" button is active, these same knobs change the step’s more complex parameters: condition and motion rules.
*   **Keypad:** It responds to button presses, allowing the user to quickly toggle the entire step on or off.

#### Real-Time Feedback

During musical playback, the component connects to the sequencing engine to receive real-time updates. If the displayed step is the one currently being played, the component visually changes its appearance (using a specific *playing color*) to give the user immediate feedback on the sequence position.

#### Customization and Setup

The component is highly flexible, allowing extensive control over its appearance using various color settings for the background, text, velocity bars, and playing indicators. Crucially, a configuration function links this visual element to the specific data streams of an underlying audio plugin, ensuring the UI accurately reads and writes the correct musical information.

sha: d394f270408efad3be82b67384e1ac46ef8e48946ad62fe83969c9d2302c8337 
*/
#ifndef _UI_PIXEL_COMPONENT_STEP_EDIT2_H_
#define _UI_PIXEL_COMPONENT_STEP_EDIT2_H_

#include "helpers/midiNote.h"
#include "plugins/audio/stepInterface.h"
#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <stdexcept>

/*md
## StepEdit2

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/stepEdit2.png" />

StepEdit component is used to edit a step value.
*/

class StepEdit2Component : public Component {
protected:
    AudioPlugin* plugin = NULL;
    Step* step;
    uint8_t* stepCounter = NULL;

    bool notePlaying = false;
    bool* seqPlayingPtr = NULL;
    bool seqPlaying = false;

    KeypadLayout keypadLayout;

    Color bgColor;
    Color selection;
    Color noteColor;
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

    uint8_t encoderId1 = 1;
    uint8_t encoderId2 = 2;
    uint8_t encoderId3 = 3;

    uint8_t shiftContextIndex = 100;

public:
    StepEdit2Component(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , selection(lighten(styles.colors.background, 0.5))
        , noteColor(styles.colors.primary)
        , note2Color(styles.colors.white)
        , text(styles.colors.text)
        , text2(darken(styles.colors.text, 0.3))
        , barBackground(darken(styles.colors.tertiary, 0.5))
        , bar(styles.colors.tertiary)
        , textMotion1(styles.colors.secondary)
        , textMotion2(styles.colors.quaternary)
        , playingColor(styles.colors.tertiary)
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == ".toggle") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        step->enabled = !step->enabled;
                        renderNext();
                    }
                };
            }
            return func;
        })
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
    }

    void renderNote(int x, int y)
    {
        const char* note = MIDI_NOTES_STR[step->note];
        const char noteLetter[2] = { note[0], '\0' };
        const char* noteSuffix = note + 1;
        x = draw.text({ x + 2, y }, noteLetter, 8, { noteColor });
        draw.text({ x - 2, y }, noteSuffix, 8, { note2Color });
    }

    void render() override
    {
        if (step) {
            Color bg = selection;
            draw.filledRect(relativePosition, size, { bg });

            int y = relativePosition.y;
            int x = relativePosition.x + 1;

            x = relativePosition.x + 12;
            draw.filledRect({ x, y + 2 }, { 50, 4 }, { barBackground });
            draw.filledRect({ x, y + 2 }, { (int)(50 * step->velocity), 4 }, { bar });

            x = relativePosition.x + 70;
            if (step->enabled) {
                renderNote(x, y);
            } else {
                draw.text({ x, y }, "---", 8, { text2 });
            }

            // TODO if 0 make infinit sign
            x = relativePosition.x + 110;
            if (!step->len) {
                draw.text({ x, y }, "O", 8, { text2 });
                draw.text({ x + 4, y }, "O", 8, { text2 });
            } else {
                draw.text({ x, y }, std::to_string(step->len) + "/32", 8, { text2 });
            }

            draw.text({ relativePosition.x + 158, y }, stepConditions[step->condition].name, 8, { text2 });

            std::string motionSteps = stepMotions[step->motion].name;
            x = relativePosition.x + 196;
            if (motionSteps == "---") {
                draw.text({ x, y }, motionSteps, 8, { text2 });
            } else {
                char* motionStep = strtok((char*)motionSteps.c_str(), ",");
                for (int i = 0; motionStep != NULL; i++) {
                    x = draw.text({ x, y }, motionStep, 8, { i % 2 == 0 ? textMotion1 : textMotion2 });
                    motionStep = strtok(NULL, ",");
                }
            }

            x = relativePosition.x + 233;
            if ((seqPlayingPtr == NULL || seqPlaying) && notePlaying) {
                draw.filledRect({ x, y + 1 }, { 6, 6 }, { playingColor });
            }
        }
    }

    void onEncoder(int8_t id, int8_t direction) override
    {
        if (id == encoderId1) {
            if (view->contextVar[shiftContextIndex]) {
                step->setCondition(step->condition + direction);
                renderNext();
            } else {
                step->setVelocity(step->velocity + direction * 0.01);
                renderNext();
            }
        } else if (id == encoderId2) {
            if (view->contextVar[shiftContextIndex]) {
                step->setMotion(step->motion + direction);
                renderNext();
            } else {
                step->setNote(step->note + direction);
                renderNext();
            }
        } else if (id == encoderId3) {
            if (view->contextVar[shiftContextIndex]) {
            } else {
                step->setLength(step->len + direction);
                renderNext();
            }
        }
    }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        keypadLayout.onKey(id, key, state, now);
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        if (keypadLayout.config(key, value)) {
            return true;
        }

        /*md - `DATA: plugin_name get_step_data_id step_index [sequence_data_id] [counter_data_id]` set plugin target */
        if (strcmp(key, "DATA") == 0) {
            plugin = &getPlugin(strtok(value, " "), track);
            stepIndex = atoi(strtok(NULL, " "));
            char* getStepDataIdStr = strtok(NULL, " ");
            char* sequenceDataIdStr = strtok(NULL, " ");
            char* counterDataIdStr = strtok(NULL, " ");
            step = (Step*)plugin->data(plugin->getDataId(getStepDataIdStr != NULL ? getStepDataIdStr : "GET_STEP"), &stepIndex);
            seqPlayingPtr = (bool*)plugin->data(plugin->getDataId(sequenceDataIdStr != NULL ? sequenceDataIdStr : "IS_PLAYING"));
            stepCounter = (uint8_t*)plugin->data(plugin->getDataId(counterDataIdStr != NULL ? counterDataIdStr : "STEP_COUNTER"));
            return true;
        }

        /*md - `ENCODERS: encoder_id1 encoder_id2 encoder_id3` is the id of the encoder to update step value. This component use 3 encoders. In standard view, it will change note, velocity and length. In shift view, it will change note, condition and motion. */
        if (strcmp(key, "ENCODERS") == 0) {
            encoderId1 = atoi(strtok(value, " "));
            encoderId2 = atoi(strtok(NULL, " "));
            encoderId3 = atoi(strtok(NULL, " "));
            return true;
        }

        /*md - `SHIFT_CONTEXT_INDEX: index` is the index of the context (default: 100). */
        if (strcmp(key, "SHIFT_CONTEXT_INDEX") == 0) {
            shiftContextIndex = atoi(value);
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(value);
            // printf("bgColor %d %d %d\n", bgColor.r, bgColor.g, bgColor.b);
            return true;
        }

        /*md - `PLAYING_COLOR: color` is the color of actual playing step. */
        if (strcmp(key, "PLAYING_COLOR") == 0) {
            playingColor = draw.getColor(value);
            return true;
        }

        /*md - `NOTE_COLOR: color` is the color of the note. */
        if (strcmp(key, "NOTE_COLOR") == 0) {
            noteColor = draw.getColor(value);
            return true;
        }

        /*md - `NOTE2_COLOR: color` is the color of the note. */
        if (strcmp(key, "NOTE2_COLOR") == 0) {
            note2Color = draw.getColor(value);
            return true;
        }

        /*md - `TEXT_COLOR: color` is the color of the text. */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            text = draw.getColor(value);
            return true;
        }

        /*md - `BAR_COLOR: color` is the color of the velocity bar. */
        if (strcmp(key, "BAR_COLOR") == 0) {
            bar = draw.getColor(value);
            barBackground = darken(bar, 0.5);
            return true;
        }

        /*md - `TEXT_MOTION1_COLOR: color` is the first color of the motion text. */
        if (strcmp(key, "TEXT_MOTION1_COLOR") == 0) {
            textMotion1 = draw.getColor(value);
            return true;
        }

        /*md - `TEXT_MOTION2_COLOR: color` is the second color of the motion text. */
        if (strcmp(key, "TEXT_MOTION2_COLOR") == 0) {
            textMotion2 = draw.getColor(value);
            return true;
        }

        /*md - `SELECTED_COLOR: color` is the color of the selected step. */
        if (strcmp(key, "SELECTED_COLOR") == 0) {
            selection = draw.getColor(value);
            return true;
        }

        return Component::config(key, value);
    }
};

#endif
