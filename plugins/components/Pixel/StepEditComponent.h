#ifndef _UI_PIXEL_COMPONENT_STEP_EDIT_H_
#define _UI_PIXEL_COMPONENT_STEP_EDIT_H_

#include "helpers/midiNote.h"
#include "plugins/audio/stepInterface.h"
#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"
#include "utils/GroupColorComponent.h"

/*md
## StepEdit

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/stepEdit.png" />

StepEdit component is used to edit a step value.
*/

// [note, velocity, len] [probabilty, condition, motion]
// For the note length, we should show the note symbol

// Push encoder scroll throught selection
// --> if shift is pressed it jump of group

// Shift + key will select step
// key only will enable/disable step (and selecting it at the same time)

// pressing encoder will toggle between basic and step probability mode
// also shift + key on an already selected step will toggle between basic and step probability mode

class StepEditComponent : public GroupColorComponent {
protected:
    AudioPlugin* plugin = NULL;
    Step* step;

    KeypadLayout keypadLayout;

    Color bgColor;
    Color selection;
    ToggleColor text;
    ToggleColor text2;
    ToggleColor barBackground;
    ToggleColor bar;
    ToggleColor textMotion1;
    ToggleColor textMotion2;

    uint8_t dataId = -1;
    uint8_t stepIndex = -1;

    bool selected = false;
    int16_t groupRange[2] = { -1, -1 };

    uint8_t encoderId1 = -1;
    uint8_t encoderId2 = -1;
    uint8_t encoderId3 = -1;

    uint8_t shiftIndex = 255;

public:
    StepEditComponent(ComponentInterface::Props props)
        : GroupColorComponent(props, { { "TEXT_COLOR", &text }, { "TEXT2_COLOR", &text2 }, { "BAR_BACKGROUND_COLOR", &barBackground }, { "BAR_COLOR", &bar }, { "TEXT_MOTION1_COLOR", &textMotion1 }, { "TEXT_MOTION2_COLOR", &textMotion2 } })
        , bgColor(styles.colors.background)
        , selection(styles.colors.primary)
        , text(styles.colors.text, inactiveColorRatio)
        , text2(darken(styles.colors.text, 0.5), inactiveColorRatio)
        , barBackground(darken(styles.colors.tertiary, 0.5), inactiveColorRatio)
        , bar(styles.colors.tertiary, inactiveColorRatio)
        , textMotion1(styles.colors.secondary, inactiveColorRatio)
        , textMotion2(styles.colors.quaternary, inactiveColorRatio)
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == "stepToggle") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        step->enabled = !step->enabled;
                        view->setGroup(group);
                        renderNext();
                    }
                };
            }
            return func;
        })
    {
        updateColors();
    }

    void onShift(uint8_t index, uint8_t value) override
    {
        if (index == shiftIndex) {
            renderNext();
        }
    }

    void renderNote(int y)
    {
        const char* note = MIDI_NOTES_STR[step->note];
        const char noteLetter[2] = { note[0], '\0' };
        const char* noteSuffix = note + 1;
        int x = draw.text({ relativePosition.x + 2, y }, noteLetter, 16, { text.color });
        draw.text({ x - 2, y + 6 }, noteSuffix, 8, { text.color });
    }

    void render() override
    {
        if (updatePosition() && step) {
            draw.filledRect(relativePosition, size, { bgColor });

            int y = relativePosition.y + (size.h - 16) * 0.5;
            if (step->enabled) {
                renderNote(y);
                if (view->shift[shiftIndex]) {
                    draw.text({ relativePosition.x + 32, y }, stepConditions[step->condition].name, 8, { text2.color });

                    std::string motionSteps = stepMotions[step->motion].name;
                    int x = relativePosition.x + 32;
                    if (motionSteps == "---") {
                        draw.text({ x, y + 8 }, motionSteps, 8, { text2.color });
                    } else {
                        char* motionStep = strtok((char*)motionSteps.c_str(), ",");
                        for (int i = 0; motionStep != NULL; i++) {
                            x = draw.text({ x, y + 8 }, motionStep, 8, { i % 2 == 0 ? textMotion1.color : textMotion2.color });
                            motionStep = strtok(NULL, ",");
                        }
                    }
                } else {
                    float centerX = relativePosition.x + size.w * 0.5;

                    int barWidth = size.w * 0.40;
                    int barX = (int)(centerX - barWidth * 0.5);
                    draw.filledRect({ barX, y }, { barWidth, 3 }, { barBackground.color });

                    if (step->velocity) {
                        draw.filledRect({ barX, y }, { (int)(barWidth * step->velocity), 3 }, { bar.color });
                    }

                    // TODO if 0 make infinit sign
                    draw.textRight({ relativePosition.x + size.w - 4, y + 6 }, std::to_string(step->len) + "/32", 8, { text2.color });
                }
            } else {
                draw.textCentered({ (int)(relativePosition.x + size.w * 0.5), y }, "---", 16, { text.color });
            }

            if (selected) {
                draw.rect(relativePosition, { size.w - 1, size.h - 1 }, { selection });
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
                } else if (view->shift[shiftIndex]) {
                    step->setCondition(step->condition + direction);
                } else {
                    step->setVelocity(step->velocity + direction * 0.05);
                }
                renderNext();
            } else if (id == encoderId3) {
                if (!step->enabled) {
                    step->enabled = true;
                } else if (view->shift[shiftIndex]) {
                    step->setMotion(step->motion + direction);
                } else {
                    step->setLength(step->len + direction);
                }
                renderNext();
            }
        }
    }

    void onGroupChanged(int8_t index) override
    {
        bool isSameGroup = group == index;
        bool isInGroupRange = groupRange[0] != -1 && index >= groupRange[0] && index <= groupRange[1];
        if (isInGroupRange != isActive) {
            isActive = isInGroupRange;
            updateColors();
            renderNext();
        }
        if (isSameGroup != selected) {
            selected = isSameGroup;
            renderNext();
        }
    }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        if (isActive) {
            keypadLayout.onKey(id, key, state, now);
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        if (keypadLayout.config(key, value)) {
            return true;
        }

        if (strcmp(key, "GROUP_RANGE") == 0) {
            groupRange[0] = atoi(strtok(value, " "));
            groupRange[1] = atoi(strtok(NULL, " "));
            return true;
        }

        /*md - `DATA: plugin_name data_id step_index` set plugin target */
        if (strcmp(key, "DATA") == 0) {
            plugin = &getPlugin(strtok(value, " "), track);
            dataId = atoi(strtok(NULL, " "));
            stepIndex = atoi(strtok(NULL, " "));
            step = (Step*)plugin->data(dataId, &stepIndex);
            return true;
        }

        /*md - `ENCODERS: encoder_id1 encoder_id2 encoder_id3` is the id of the encoder to update step value. This component use 3 encoders. In standard view, it will change note, velocity and length. In shift view, it will change note, condition and motion. */
        if (strcmp(key, "ENCODERS") == 0) {
            encoderId1 = atoi(strtok(value, " "));
            encoderId2 = atoi(strtok(NULL, " "));
            encoderId3 = atoi(strtok(NULL, " "));
            return true;
        }

        /*md - `SHIFT_INDEX: 255` set the index of the shift bank to use. There is 255 shift banks share within the whole app. */
        if (strcmp(key, "SHIFT_INDEX") == 0) {
            shiftIndex = atoi(value);
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(value);
            return true;
        }

        /*md - `TEXT_COLOR: color` is the color of the text. */
        /*md - `BAR_COLOR: color` is the color of the velocity bar. */
        /*md - `BAR_BACKGROUND_COLOR: color` is the color of the velocity bar background. */
        /*md - `TEXT_MOTION1_COLOR: color` is the first color of the motion text. */
        /*md - `TEXT_MOTION2_COLOR: color` is the second color of the motion text. */
        return GroupColorComponent::config(key, value);
    }
};

#endif
