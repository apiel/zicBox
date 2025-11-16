#ifndef _UI_PIXEL_COMPONENT_STEP_EDIT_DRUM_H_
#define _UI_PIXEL_COMPONENT_STEP_EDIT_DRUM_H_

#include "helpers/midiNote.h"
#include "plugins/audio/stepInterface.h"
#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <stdexcept>

/*md
## StepEditMono

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/stepEditSmall.png" />

StepEditMono component is used to edit a step value using only one knob.
*/
// NOTE should this allow to edit semitone and motion
// -12 to +12 semitone and motion...?

class StepEditMonoComponent : public Component {
protected:
    AudioPlugin* plugin = NULL;
    Step* step;
    uint8_t* stepCounter = NULL;

    bool notePlaying = false;
    bool* seqPlayingPtr = NULL;
    bool seqPlaying = false;
    bool showPlayingStep = false;

    KeypadLayout keypadLayout;

    Color bgColor;
    Color selection;
    Color onColor;
    Color conditionColor;
    Color offColor;
    Color playingColor;

    uint8_t stepIndex = -1;

    uint8_t encoder = 0;

public:
    StepEditMonoComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , selection(lighten(styles.colors.background, 0.5))
        , onColor(styles.colors.primary)
        , offColor(darken(styles.colors.text, 0.3))
        , conditionColor(styles.colors.text)
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
            if (showPlayingStep) {
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
            }
        };
    }

    void render() override
    {
        if (step) {
            Color bg = selection;
            draw.filledRect(relativePosition, size, { bg });

            int y = relativePosition.y;
            int x = relativePosition.x;

            if (!step->enabled) {
                draw.text({ x, y }, "---", 8, { offColor });
            } else if (step->condition) {
                draw.text({ x, y }, stepConditions[step->condition].name, 8, { conditionColor });
            } else {
                draw.text({ x, y }, "on", 8, { onColor });
            }

            if (showPlayingStep && (seqPlayingPtr == NULL || seqPlaying) && notePlaying) {
                draw.filledRect({ x + size.w - 10, y + 1 }, { 6, 6 }, { playingColor });
            }
        }
    }

    void onEncoder(int8_t id, int8_t direction) override
    {
        if (id == encoder) {
            if (direction > 0) {
                if (!step->enabled) {
                    step->enabled = true;
                    renderNext();
                } else {
                    step->setCondition(step->condition + direction);
                    renderNext();
                }
            } else if (direction < 0) {
                if (step->condition) {
                    step->setCondition(step->condition + direction);
                    renderNext();
                } else {
                    step->enabled = false;
                    renderNext();
                }
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

        /*md - `ENCODER: encoder_id` is the id of the encoder to update step value. */
        if (strcmp(key, "ENCODER") == 0) {
            encoder = atoi(value);
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

        /*md - `SHOW_PLAYING_STEP: true` is to enable the task to show actual playing step. */
        if (strcmp(key, "SHOW_PLAYING_STEP") == 0) {
            if (strcmp(value, "true") == 0) {
                showPlayingStep = true;
            }
            return true;
        }

        /*md - `ON_COLOR: color` is the color of the note. */
        if (strcmp(key, "ON_COLOR") == 0) {
            onColor = draw.getColor(value);
            return true;
        }

        /*md - `OFF_COLOR: color` is the color of the text. */
        if (strcmp(key, "OFF_COLOR") == 0) {
            offColor = draw.getColor(value);
            return true;
        }

        /*md - `CONDITION_COLOR: color` is the color of the text. */
        if (strcmp(key, "CONDITION_COLOR") == 0) {
            conditionColor = draw.getColor(value);
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
