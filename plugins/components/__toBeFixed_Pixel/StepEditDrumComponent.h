#ifndef _UI_PIXEL_COMPONENT_STEP_EDIT_DRUM_H_
#define _UI_PIXEL_COMPONENT_STEP_EDIT_DRUM_H_

#include "helpers/midiNote.h"
#include "plugins/audio/stepInterface.h"
#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <stdexcept>

/*md
## StepEditDrum

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/stepEditDrum.png" />

StepEditDrum component is used to edit a step value for drums.
*/

class StepEditDrumComponent : public Component {
protected:
    AudioPlugin* plugin = NULL;
    Step* step;
    uint16_t* stepCounter = NULL;

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

    uint16_t stepIndex = -1;

    uint8_t encoders[4] = { 0, 1, 2, 3 };

public:
    StepEditDrumComponent(ComponentInterface::Props props)
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

        Step* _step = step;

        Step dummyStep;
        if (_step == NULL) {
            _step = &dummyStep;
        }

        if (_step) {
            Color bg = selection;
            draw.filledRect(relativePosition, size, { bg });

            int y = relativePosition.y;
            int x = relativePosition.x + 1;

            x = relativePosition.x + 12;
            draw.filledRect({ x, y + 2 }, { 50, 4 }, { barBackground });
            draw.filledRect({ x, y + 2 }, { (int)(50 * _step->velocity), 4 }, { bar });

            x = relativePosition.x + 70;
            if (_step->enabled) {
                renderNote(x, y);
            } else {
                draw.text({ x, y }, "---", 8, { text2 });
            }

            draw.text({ relativePosition.x + 110, y }, stepConditions[_step->condition].name, 8, { text2 });

            std::string motionSteps = stepMotions[_step->motion].name;
            x = relativePosition.x + 156;
            if (motionSteps == "---") {
                draw.text({ x, y }, motionSteps, 8, { text2 });
            } else {
                char* motionStep = strtok((char*)motionSteps.c_str(), ",");
                for (int i = 0; motionStep != NULL; i++) {
                    x = draw.text({ x, y }, motionStep, 8, { i % 2 == 0 ? textMotion1 : textMotion2 });
                    motionStep = strtok(NULL, ",");
                }
            }

            x = relativePosition.x + 193;
            if ((seqPlayingPtr == NULL || seqPlaying) && notePlaying) {
                draw.filledRect({ x, y + 1 }, { 6, 6 }, { playingColor });
            }
        }
    }

    void onEncoder(int8_t id, int8_t direction) override
    {
        if (step == NULL && (id == encoders[0] || id == encoders[1] || id == encoders[2] || id == encoders[3])) {
            printf("[StepEditDrumComponent] step is NULL, not gonna fix it as it should be deprecated\n");
            return;
        }
        if (id == encoders[0]) {
            step->setVelocity(step->velocity + direction * 0.01);
            renderNext();
        } else if (id == encoders[1]) {
            step->setNote(step->note + direction);
            renderNext();
        } else if (id == encoders[2]) {
            step->setCondition(step->condition + direction);
            renderNext();
        } else if (id == encoders[3]) {
            step->setMotion(step->motion + direction);
            renderNext();
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
            stepCounter = (uint16_t*)plugin->data(plugin->getDataId(counterDataIdStr != NULL ? counterDataIdStr : "STEP_COUNTER"));
            return true;
        }

        /*md - `ENCODERS: encoder_id1 encoder_id2 encoder_id3 encoder_id4` is the id of the encoder to update step value. */
        if (strcmp(key, "ENCODERS") == 0) {
            encoders[0] = atoi(strtok(value, " "));
            encoders[1] = atoi(strtok(NULL, " "));
            encoders[2] = atoi(strtok(NULL, " "));
            encoders[3] = atoi(strtok(NULL, " "));
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
