#ifndef _UI_PIXEL_COMPONENT_SYNTH_BAR_H_
#define _UI_PIXEL_COMPONENT_SYNTH_BAR_H_

#include "helpers/midiNote.h"
#include "plugins/audio/stepInterface.h"
#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

/*md
## SynthBar

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/synthBar.png" />

*/

class SynthBarComponent : public Component {
protected:
    bool isActive = true;

    KeypadLayout keypadLayout;

    Color background;
    Color selectionColor;
    Color textColor;
    Color foreground;

    int fontSize = 6;

    uint8_t itemIndex = -1;

    uint8_t selectedItemBank = 10;

    struct Item {
        std::string name;
    };

    std::vector<Item> items = { { "Main" }, { "FX" }, { "Amp." }, { "Freq." }, { "Wave"}, { "Clic"} };

public:
    SynthBarComponent(ComponentInterface::Props props)
        : Component(props)
        , background(styles.colors.background)
        , selectionColor(styles.colors.white)
        , textColor(styles.colors.text)
        , foreground({ 0x30, 0x30, 0x30 })
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
                    // if (KeypadLayout::isReleased(keymap)) {
                    //     if (view->contextVar[selectedItemBank] < stepCount) {
                    //         setContext(selectedItemBank, view->contextVar[selectedItemBank] + 1);
                    //     }
                    //     renderNext();
                    // }
                };
            }
            if (action == ".stepToggle") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    // if (KeypadLayout::isReleased(keymap)) {
                    //     if (view->contextVar[selectedItemBank] > 0) {
                    //         seqStepEnabled->set(!seqStepEnabled->get());
                    //         renderNext();
                    //     } else {
                    //         seqStatus->set(seqStatus->get() == 0 ? 1 : 0);
                    //     }
                    // }
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
                // seqSelectedStep->set((int)(view->contextVar[selectedItemBank]));
            }
        }
    }

    void render() override
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { background });

            int stepH = size.h;

            int textY = (size.h - 8) * 0.5 + relativePosition.y;
            int itemW = size.w / items.size();

            int x = relativePosition.x + 1;
            for (int i = 0; i < items.size(); i++) {
                Item item = items[i];
                // draw.filledRect({ x, relativePosition.y }, { itemW, stepH }, { foreground });
                draw.text({ x + 2, textY }, item.name, fontSize, { textColor });
                if (isActive && view->contextVar[selectedItemBank] == i + 1) {
                    draw.rect({ x, relativePosition.y }, { itemW, stepH - 1 }, { selectionColor });
                }
                x += itemW + 2;
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

        /*md - `SELECTION_COLOR: color` is the selection color. */
        if (strcmp(key, "SELECTION_COLOR") == 0) {
            selectionColor = draw.getColor(value);
            return true;
        }

        return false;
    }
};

#endif
