#ifndef _UI_PIXEL_COMPONENT_STEP_EDIT_SAMPLE_H_
#define _UI_PIXEL_COMPONENT_STEP_EDIT_SAMPLE_H_

#include "helpers/format.h"
#include "helpers/midiNote.h"
#include "plugins/audio/SampleStep.h"
#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <filesystem>
#include <stdexcept>

/*md
## StepEditSample

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/stepEditSample.png" />

StepEditSample component is used to edit a step value for samples.
*/

class StepEditSampleComponent : public Component {
protected:
    bool isActive = true;

    AudioPlugin* plugin = NULL;
    SampleStep* step;
    uint8_t* stepCounter = NULL;

    uint8_t nextFileDataId = 0;
    uint8_t prevFileDataId = 0;
    uint8_t playStepDataId = 0;

    bool notePlaying = false;
    bool* seqPlayingPtr = NULL;
    bool seqPlaying = false;

    KeypadLayout keypadLayout;

    Color bgColor;
    Color selection;
    Color fileColor;
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

    uint8_t encoders[4] = { 0, 1, 2, 3 };

public:
    StepEditSampleComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , selection(lighten(styles.colors.background, 0.5))
        , fileColor(styles.colors.primary)
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
            if (action == ".play") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        plugin->data(playStepDataId, &stepIndex);
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

    int renderPct(int x, int y, float pct)
    {
        x = draw.text({ x + 2, y }, std::to_string((int)(pct)), 8, { text });
        draw.text({ x - 4, y }, ".", 8, { text });
        int decimal = (pct - (int)pct) * 10;
        x = draw.text({ x + 2, y }, std::to_string(decimal), 8, { text2 });
        x = draw.text({ x, y + 4 }, "%", 4, { text2 });

        return x;
    }

    void renderStartEnd(int y, Color bg)
    {
        int x = relativePosition.x + size.w - 84;
        draw.filledRect({ x, y }, { 84, 8 }, { bg });

        renderPct(x + 2, y, step->fStart);
        renderPct(x + 42, y, step->fEnd);
    }

    void render() override
    {
        if (updatePosition() && step) {
            Color bg = isActive ? selection : bgColor;
            draw.filledRect(relativePosition, size, { bg });

            int y = relativePosition.y;
            int x = relativePosition.x + 1;

            x = relativePosition.x + 2;
            draw.filledRect({ x, y + 2 }, { 20, 4 }, { barBackground });
            draw.filledRect({ x, y + 2 }, { (int)(20 * step->velocity), 4 }, { bar });

            x = relativePosition.x + 25;
            if (step->enabled) {
                draw.text({ x, y }, std::filesystem::path(step->filename).filename(), 8, { fileColor });

                // if (step->fStart != 0 || step->fEnd != 100) {
                renderStartEnd(y, bg);
                // }
            } else {
                draw.text({ x, y }, "---", 8, { text2 });
            }
        }
    }

    void onEncoder(int id, int8_t direction) override
    {
        if (isActive) {
            if (id == encoders[0]) {
                step->setVelocity(step->velocity + direction * 0.05);
                renderNext();
            } else if (id == encoders[2]) {
                if (direction > 0) {
                    plugin->data(nextFileDataId, &stepIndex);
                } else {
                    plugin->data(prevFileDataId, &stepIndex);
                }
                renderNext();
            } else if (id == encoders[1]) {
                step->setStart(step->fStart + direction * 0.1);
                renderNext();
            } else if (id == encoders[3]) {
                step->setEnd(step->fEnd + direction * 0.1);
                renderNext();
            }
        }
    }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        if (isActive) {
            keypadLayout.onKey(id, key, state, now);
        }
    }

    void onGroupChanged(int8_t index) override
    {
        bool shouldActivate = false;
        if (group == index || group == -1) {
            shouldActivate = true;
        }
        if (shouldActivate != isActive) {
            isActive = shouldActivate;
            renderNext();
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        if (keypadLayout.config(key, value)) {
            return true;
        }

        /*md - `DATA: plugin_name step_index [get_step_data_id]` set plugin target */
        if (strcmp(key, "DATA") == 0) {
            plugin = &getPlugin(strtok(value, " "), track);
            stepIndex = atoi(strtok(NULL, " "));
            char* getStepDataIdStr = strtok(NULL, " ");

            step = (SampleStep*)plugin->data(plugin->getDataId(getStepDataIdStr != NULL ? getStepDataIdStr : "GET_STEP"), &stepIndex);
            nextFileDataId = plugin->getDataId("NEXT_FILE");
            prevFileDataId = plugin->getDataId("PREVIOUS_FILE");
            playStepDataId = plugin->getDataId("PLAY_STEP");
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

        /*md - `FILE_COLOR: color` is the color of the file. */
        if (strcmp(key, "FILE_COLOR") == 0) {
            fileColor = draw.getColor(value);
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
