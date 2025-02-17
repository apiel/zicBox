#ifndef _UI_PIXEL_COMPONENT_SEQ_PROGRESS_BAR_H_
#define _UI_PIXEL_COMPONENT_SEQ_PROGRESS_BAR_H_

#include "helpers/midiNote.h"
#include "plugins/audio/stepInterface.h"
#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

/*md
## SeqProgressBarComponent

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/seqProgressBar.png" />

*/

class SeqProgressBarComponent : public Component {
protected:
    bool isActive = true;

    uint16_t stepCount = 32;
    int16_t lastStepCounter = -1;
    uint16_t* stepCounter = NULL;
    bool* seqPlayingPtr = NULL;
    Step* steps = NULL;

    KeypadLayout keypadLayout;

    Color background;
    Color selectionColor;
    Color foreground;
    Color activeColor;
    Color inactiveStepColor;

    ValueInterface* valVolume = NULL;
    bool showSteps = false;

public:
    SeqProgressBarComponent(ComponentInterface::Props props)
        : Component(props)
        , background(styles.colors.background)
        , selectionColor(styles.colors.white)
        , foreground(darken(styles.colors.primary, 0.5))
        , activeColor(styles.colors.primary)
        , inactiveStepColor({ 0x40, 0x40, 0x40 })
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            return func;
        })
    {
        jobRendering = [this](unsigned long now) {
            if (seqPlayingPtr != NULL) {
                if (*seqPlayingPtr) {
                    if (stepCounter != NULL && lastStepCounter != *stepCounter) {
                        lastStepCounter = *stepCounter;
                        renderNext();
                    }
                } else {
                    if (lastStepCounter != -1) {
                        renderNext();
                    }
                    lastStepCounter = -1;
                }
            }
        };
    }

    void render() override
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { background });

            int stepW = 4;
            int stepH = size.h;

            int textY = (size.h - 8) * 0.5 + relativePosition.y;

            int stepsW = stepCount * (stepW + 2 + 0.5); // 2 / 4 adding 2 pixel every 4 steps
            int nameW = size.w - stepsW - 5;
            int x = relativePosition.x + 1;

            if (valVolume != NULL) {
                draw.filledRect({ x, relativePosition.y }, { nameW, stepH }, { darken(activeColor, 0.5) });
                draw.filledRect({ x, relativePosition.y }, { (int)(nameW * valVolume->pct()), stepH }, { activeColor });
                if (isActive) {
                    draw.rect({ x, relativePosition.y }, { nameW, stepH - 1 }, { selectionColor });
                }
            }

            x += nameW + 4;

            for (int i = 0; i < stepCount; i++) {
                Color color = lastStepCounter == i ? activeColor : (!showSteps || steps[i].enabled ? foreground : inactiveStepColor);
                draw.filledRect({ x, relativePosition.y }, { stepW, stepH }, { color });
                x += stepW + 2;
                if (i % 4 == 3) {
                    x += 2;
                }
            }
        }
    }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now) override
    {
        if (isActive) {
            keypadLayout.onKey(id, key, state, now);
        }
    }

    void onGroupChanged(int8_t index) override
    {
        if (isActive) {
            renderNext();
        }

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

        /*md - `SEQ_PLUGIN: plugin_name [track]` set plugin target for sequencer */
        if (strcmp(key, "SEQ_PLUGIN") == 0) {
            AudioPlugin* seqPlugin = NULL;

            char* pluginName = strtok(value, " ");
            char* trackStr = strtok(NULL, " ");
            int trackId = trackStr == NULL ? track : atoi(trackStr);
            seqPlugin = &getPlugin(pluginName, trackId);
            // stepCount = seqPlugin->getValue("SELECTED_STEP")->props().max;
            stepCount = *(uint16_t*)seqPlugin->data(seqPlugin->getDataId("STEP_COUNT"));
            stepCounter = (uint16_t*)seqPlugin->data(seqPlugin->getDataId("STEP_COUNTER"));
            seqPlayingPtr = (bool*)seqPlugin->data(seqPlugin->getDataId("IS_PLAYING"));
            steps = (Step*)seqPlugin->data(seqPlugin->getDataId("STEPS"));

            return true;
        }

        /*md - `VOLUME_PLUGIN: plugin_name value_key` is used for the volume bar (but can be any else). */
        if (strcmp(key, "VOLUME_PLUGIN") == 0) {
            valVolume = watch(getPlugin(strtok(value, " "), track).getValue(strtok(NULL, " ")));
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            background = draw.getColor(value);
            return true;
        }

        /*md - `FOREGROUND_COLOR: color` is the foreground color. */
        if (strcmp(key, "FOREGROUND_COLOR") == 0) {
            foreground = draw.getColor(value);
            return true;
        }

        /*md - `ACTIVE_COLOR: color` is the color of the active step. */
        if (strcmp(key, "ACTIVE_COLOR") == 0) {
            activeColor = draw.getColor(value);
            return true;
        }

        /*md - `SELECTION_COLOR: color` is the selection color. */
        if (strcmp(key, "SELECTION_COLOR") == 0) {
            selectionColor = draw.getColor(value);
            return true;
        }

        /*md - `SHOW_STEPS: true` show sequencer step value (default: false). */
        if (strcmp(key, "SHOW_STEPS") == 0) {
            showSteps = strcmp(value, "true") == 0;
            return true;
        }

        return false;
    }
};

#endif
