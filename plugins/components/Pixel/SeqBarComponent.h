#ifndef _UI_PIXEL_COMPONENT_SEQ_BAR_H_
#define _UI_PIXEL_COMPONENT_SEQ_BAR_H_

#include "helpers/midiNote.h"
#include "plugins/audio/stepInterface.h"
#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

/*md
## SeqBar

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/seqBar.png" />

*/

class SeqBarComponent : public Component {
protected:
    bool isActive = true;

    Step* steps = NULL;
    uint8_t stepCount = 32;

    KeypadLayout keypadLayout;

    Color background;
    Color selectionColor;
    Color textColor;
    Color foreground;
    Color activeStepColor;
    Color nameColor;
    Color arrowColor;

    uint8_t stepIndex = -1;

    std::string name;

    ValueInterface* valName = NULL;
    ValueInterface* valVolume = NULL;
    ValueInterface* seqStatus = NULL;
    ValueInterface* seqSelectedStep = NULL; // SELECTED_STEP
    ValueInterface* seqStepEnabled = NULL; // STEP_ENABLED

    uint8_t selectedItemBank = 10;

    int stepW = 4;

public:
    SeqBarComponent(ComponentInterface::Props props)
        : Component(props)
        , background(styles.colors.background)
        , selectionColor(styles.colors.white)
        , textColor(styles.colors.text)
        , foreground({ 0x40, 0x40, 0x40 })
        , arrowColor({ 0x80, 0x80, 0x80 })
        , activeStepColor(styles.colors.primary)
        , nameColor(styles.colors.primary)
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == ".left") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (view->contextVar[selectedItemBank] > 0) {
                            setContext(selectedItemBank, view->contextVar[selectedItemBank] - 1);
                        }
                        renderNext();
                    }
                };
            }
            if (action == ".right") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (view->contextVar[selectedItemBank] < stepCount) {
                            setContext(selectedItemBank, view->contextVar[selectedItemBank] + 1);
                        }
                        renderNext();
                    }
                };
            }
            if (action == ".stepToggle") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (view->contextVar[selectedItemBank] > 0) {
                            seqStepEnabled->set(!seqStepEnabled->get());
                            renderNext();
                        } else {
                            seqStatus->set(seqStatus->get() == 0 ? 1 : 0);
                        }
                    }
                };
            }
            return func;
        })
    {
    }

    void onContext(uint8_t index, float value) override
    {
        if (index == selectedItemBank) {
            // renderNext(); // no need render
            if (view->contextVar[selectedItemBank] > 0) {
                seqSelectedStep->set((int)(view->contextVar[selectedItemBank]));
            }
        }
    }

    int rendername(int x)
    {
        int stepsW = stepCount * (stepW + 2 + 0.5); // 2 / 4 adding 2 pixel every 4 steps
        int witdhLeft = size.w - stepsW - 5;

        int nameX = x;
        int nameW = witdhLeft;

        bool showVolume = seqStatus != NULL && seqStatus->get() == 1 && valVolume != NULL;
        draw.filledRect({ nameX, relativePosition.y }, { nameW, size.h }, { showVolume ? darken(nameColor, 0.5) : foreground });
        if (showVolume) {
            draw.filledRect({ nameX, relativePosition.y }, { (int)(nameW * valVolume->pct()), size.h }, { nameColor });
        }

        int textY = (size.h - 8) * 0.5 + relativePosition.y;
        if (valName != NULL) {
            draw.text({ nameX + 2, textY }, valName->string(), 8, { textColor, .maxWidth = (nameW - 2) });
        } else if (name.length() > 0) {
            draw.text({ nameX + 2, textY }, name, 8, { textColor, .maxWidth = (nameW - 2) });
        }
        if (isActive && view->contextVar[selectedItemBank] == 0) {
            draw.rect({ nameX, relativePosition.y }, { nameW, size.h - 1 }, { selectionColor });
        }

        return witdhLeft + 4;
    }

    void render() override
    {
        if (updatePosition() && steps) {
            draw.filledRect(relativePosition, size, { background });

            int x = relativePosition.x + 1;
            x += rendername(x);
            for (int i = 0; i < stepCount; i++) {
                Step* step = &steps[i];
                Color color = step->enabled ? darken(activeStepColor, 1.0 - step->velocity) : foreground;
                draw.filledRect({ x, relativePosition.y }, { stepW, size.h }, { color });
                if (isActive && view->contextVar[selectedItemBank] == i + 1) {
                    draw.rect({ x, relativePosition.y }, { stepW, size.h - 1 }, { selectionColor });
                }
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

        /*md - `SEQ_PLUGIN: plugin_name [get_steps_data_id]` set plugin target for sequencer */
        if (strcmp(key, "SEQ_PLUGIN") == 0) {
            AudioPlugin* seqPlugin = NULL;

            seqPlugin = &getPlugin(strtok(value, " "), track);
            stepCount = seqPlugin->getValue("SELECTED_STEP")->props().max;
            seqStatus = watch(seqPlugin->getValue("STATUS"));
            seqSelectedStep = seqPlugin->getValue("SELECTED_STEP");
            seqStepEnabled = watch(seqPlugin->getValue("STEP_ENABLED"));
            watch(seqPlugin->getValue("STEP_VELOCITY")); // Watch step velocity to update step color base on velocity

            char* getStepsDataId = strtok(NULL, " ");
            uint8_t dataId = seqPlugin->getDataId(getStepsDataId != NULL ? getStepsDataId : "STEPS");
            steps = (Step*)seqPlugin->data(dataId);
            return true;
        }

        /*md - `NAME_PLUGIN: plugin_name value_key` set the plugin target to be used for the name. */
        if (strcmp(key, "NAME_PLUGIN") == 0) {
            valName = watch(getPlugin(strtok(value, " "), track).getValue(strtok(NULL, " ")));
            return true;
        }

        /*md - `NAME: name` set the name of the component. */
        if (strcmp(key, "NAME") == 0) {
            name = value;
            return true;
        }

        /*md - `VOLUME_PLUGIN: plugin_name value_key` is used for the volume bar (but can be any else). */
        if (strcmp(key, "VOLUME_PLUGIN") == 0) {
            valVolume = watch(getPlugin(strtok(value, " "), track).getValue(strtok(NULL, " ")));
            return true;
        }

        /*md - `SELECT_ITEM_CONTEXT: context_id` is the context id for the selected item (default is 10). */
        if (strcmp(key, "SELECT_ITEM_CONTEXT") == 0) {
            selectedItemBank = atoi(value);
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            background = draw.getColor(value);
            return true;
        }

        /*md - `TEXT_COLOR: color` is the color of the text. */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            textColor = draw.getColor(value);
            return true;
        }

        /*md - `FOREGROUND_COLOR: color` is the foreground color. */
        if (strcmp(key, "FOREGROUND_COLOR") == 0) {
            foreground = draw.getColor(value);
            return true;
        }

        /*md - `ACTIVE_STEP_COLOR: color` is the color of the active step. */
        if (strcmp(key, "ACTIVE_STEP_COLOR") == 0) {
            activeStepColor = draw.getColor(value);
            return true;
        }

        /*md - `NAME_COLOR: color` is the color of the name. */
        if (strcmp(key, "NAME_COLOR") == 0) {
            nameColor = draw.getColor(value);
            return true;
        }

        /*md - `SELECTION_COLOR: color` is the selection color. */
        if (strcmp(key, "SELECTION_COLOR") == 0) {
            selectionColor = draw.getColor(value);
            return true;
        }

        return false;
    }
};

#endif