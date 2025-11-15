#pragma once

#include "helpers/midiNote.h"
#include "plugins/audio/stepInterface.h"
#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

/*md
## StepEdit

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/stepEdit.png" />

StepEdit component is used to edit a step value.
*/

class StepEditComponent : public Component {
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
    Color text;
    Color text2;
    Color barBackground;
    Color bar;
    Color textMotion1;
    Color textMotion2;

    uint8_t stepIndex = -1;

    bool selected = false;
    int16_t groupRange[2] = { -1, -1 };

    uint8_t encoderId1 = -1;
    uint8_t encoderId2 = -1;
    uint8_t encoderId3 = -1;

    uint8_t shiftModeIndex = 255;
    uint8_t globalShift = 0;

    int16_t restoreShiftMode = -1;

public:
    StepEditComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , selection(styles.colors.primary)
        , text(styles.colors.text)
        , text2(darken(styles.colors.text, 0.5))
        , barBackground(darken(styles.colors.tertiary, 0.5))
        , bar(styles.colors.tertiary)
        , textMotion1(styles.colors.secondary)
        , textMotion2(styles.colors.quaternary)
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == ".toggle") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (!view->contextVar[globalShift]) {
                            step->enabled = !step->enabled;
                        } else {
                            restoreShiftMode = view->contextVar[shiftModeIndex];
                        }
                        view->setGroup(group);
                        renderNext();
                    }
                };
            }

            if (action == ".restoreShiftMode") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (restoreShiftMode != -1) {
                            view->contextVar[shiftModeIndex] = restoreShiftMode;
                            restoreShiftMode = -1;
                            renderNext();
                        }
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
            if (seqPlayingPtr == NULL || seqPlaying) {
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

    void onContext(uint8_t index, float value) override
    {
        if (index == shiftModeIndex) {
            renderNext();
        }
    }

    void renderNote(int y)
    {
        const char* note = MIDI_NOTES_STR[step->note];
        const char noteLetter[2] = { note[0], '\0' };
        const char* noteSuffix = note + 1;
        int x = draw.text({ relativePosition.x + 2, y }, noteLetter, 16, { text });
        draw.text({ x - 2, y + 6 }, noteSuffix, 8, { text });
    }

    void render() override
    {
        if (step) {
            draw.filledRect(relativePosition, size, { bgColor });

            int y = relativePosition.y + (size.h - 16) * 0.5;
            if (step->enabled) {
                renderNote(y);
                if (view->contextVar[shiftModeIndex]) {
                    draw.text({ relativePosition.x + 32, y }, stepConditions[step->condition].name, 8, { text2 });

                    std::string motionSteps = stepMotions[step->motion].name;
                    int x = relativePosition.x + 32;
                    if (motionSteps == "---") {
                        draw.text({ x, y + 8 }, motionSteps, 8, { text2 });
                    } else {
                        char* motionStep = strtok((char*)motionSteps.c_str(), ",");
                        for (int i = 0; motionStep != NULL; i++) {
                            x = draw.text({ x, y + 8 }, motionStep, 8, { i % 2 == 0 ? textMotion1 : textMotion2 });
                            motionStep = strtok(NULL, ",");
                        }
                    }
                } else {
                    float centerX = relativePosition.x + size.w * 0.5;

                    int barWidth = size.w * 0.40;
                    int barX = (int)(centerX - barWidth * 0.5);
                    draw.filledRect({ barX, y }, { barWidth, 3 }, { barBackground });

                    if (step->velocity) {
                        draw.filledRect({ barX, y }, { (int)(barWidth * step->velocity), 3 }, { bar });
                    }

                    // TODO if 0 make infinit sign
                    if (!step->len) {
                        // draw.textRight({ relativePosition.x + size.w - 4, y + 6 }, "OO", 8, { text2 });
                        draw.textRight({ relativePosition.x + size.w - 4, y + 6 }, "O", 8, { text2 });
                        draw.textRight({ relativePosition.x + size.w - 4 - 5, y + 6 }, "O", 8, { text2 });
                    } else {
                        draw.textRight({ relativePosition.x + size.w - 4, y + 6 }, std::to_string(step->len) + "/32", 8, { text2 });
                    }
                }
            } else {
                draw.textCentered({ (int)(relativePosition.x + size.w * 0.5), y }, "---", 16, { text });
            }

            if (selected) {
                draw.rect(relativePosition, { size.w - 1, size.h - 1 }, { selection });
            }

            if ((seqPlayingPtr == NULL || seqPlaying) && notePlaying) {
                // draw.filledRect({ relativePosition.x, relativePosition.y + size.h - 2 }, { size.w, 2 }, { bar });
                // draw.filledRect({ relativePosition.x + (int)(size.w * 0.25), relativePosition.y + size.h - 2 }, { (int)(size.w * 0.5), 2 }, { bar });
                draw.filledRect({ relativePosition.x + 2, relativePosition.y + size.h - 4 }, { 16, 4 }, { bar });
            }
        }
    }

    void onEncoder(int id, int8_t direction) override
    {
        if (selected) {
            if (id == encoderId1) {
                if (!step->enabled) {
                    step->enabled = true;
                } else {
                    step->setNote(step->note + direction);
                }
                renderNext();
            } else if (id == encoderId2) {
                if (!step->enabled) {
                    step->enabled = true;
                } else if (view->contextVar[shiftModeIndex]) {
                    step->setCondition(step->condition + direction);
                } else {
                    step->setVelocity(step->velocity + direction * 0.05);
                }
                renderNext();
            } else if (id == encoderId3) {
                if (!step->enabled) {
                    step->enabled = true;
                } else if (view->contextVar[shiftModeIndex]) {
                    step->setMotion(step->motion + direction);
                } else {
                    step->setLength(step->len + direction);
                }
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

        /*md - `GROUP_RANGE: index1 index2` is the index of the first and last group for selection.*/
        if (strcmp(key, "GROUP_RANGE") == 0) {
            groupRange[0] = atoi(strtok(value, " "));
            groupRange[1] = atoi(strtok(NULL, " "));
            return true;
        }

        /*md - `DATA: plugin_name data_id step_index` set plugin target */
        if (strcmp(key, "DATA") == 0) {
            plugin = &getPlugin(strtok(value, " "), track);
            uint8_t dataId = plugin->getDataId(strtok(NULL, " "));
            stepIndex = atoi(strtok(NULL, " "));
            step = (Step*)plugin->data(dataId, &stepIndex);
            return true;
        }

        /*md - `COUNTER_DATA_ID: data_id` is the data id to show active step. */
        if (strcmp(key, "COUNTER_DATA_ID") == 0) {
            stepCounter = (uint8_t*)plugin->data(plugin->getDataId(value));
            return true;
        }

        /*md - `SEQUENCE_DATA_ID: data_id` is the data id to show if the sequence is playing. */
        if (strcmp(key, "SEQUENCE_DATA_ID") == 0) {
            seqPlayingPtr = (bool*)plugin->data(plugin->getDataId(value));
            return true;
        }

        /*md - `ENCODERS: encoder_id1 encoder_id2 encoder_id3` is the id of the encoder to update step value. This component use 3 encoders. In standard view, it will change note, velocity and length. In shift view, it will change note, condition and motion. */
        if (strcmp(key, "ENCODERS") == 0) {
            encoderId1 = atoi(strtok(value, " "));
            encoderId2 = atoi(strtok(NULL, " "));
            encoderId3 = atoi(strtok(NULL, " "));
            return true;
        }

        /*md - `SHIFT_MODE: 255` set the index of the context variable bank to use to switch between velocity/length and condition/motion. There is 255 context variable banks share within the whole app. */
        if (strcmp(key, "SHIFT_MODE") == 0) {
            shiftModeIndex = atoi(value);
            return true;
        }

        /*md - `GLOBAL_SHIFT: 0` set the index of the context variable bank to detect gloabl shift value. */
        if (strcmp(key, "GLOBAL_SHIFT") == 0) {
            globalShift = atoi(value);
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(value);
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
            return true;
        }

        /*md - `BAR_BACKGROUND_COLOR: color` is the color of the velocity bar background. */
        if (strcmp(key, "BAR_BACKGROUND_COLOR") == 0) {
            barBackground = draw.getColor(value);
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

        return false;
    }
};
