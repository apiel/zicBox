#ifndef _UI_PIXEL_COMPONENT_KEYBOARD_H_
#define _UI_PIXEL_COMPONENT_KEYBOARD_H_

#include "plugins/components/base/Icon.h"
#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/VisibilityContext.h"
#include "plugins/components/utils/VisibilityGroup.h"
#include "plugins/components/utils/color.h"

#include <cmath>
#include <string>
#include <vector>

/*md
## Keyboard

Keyboard components to display keyboard.
*/

class KeyboardComponent : public Component {
protected:
    Icon icon;
    KeypadLayout keypadLayout;

    Color bgColor;
    Color itemBackground;
    Color selectionColor;
    Color textColor;

    Size itemSize = { 20, 20 };
    Point textPos = { 0, 0 };

    int selection = 0;

public:
    KeyboardComponent(ComponentInterface::Props props)
        : Component(props)
        , icon(props.view->draw)
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
        , itemBackground(lighten(styles.colors.background, 0.5))
        , selectionColor(styles.colors.primary)
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == ".left") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        selection--;
                        if (selection < 0) {
                            selection = keys.size() - 1;
                        }
                        renderNext();
                    }
                };
            }
            if (action == ".right") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        selection++;
                        if (selection >= keys.size()) {
                            selection = 0;
                        }
                        renderNext();
                    }
                };
            }
            if (action == ".up") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        int nextSelection = selection - 9;
                        if (nextSelection >= 0) {
                            selection = nextSelection;
                        }
                        
                        renderNext();
                    }
                };
            }
            if (action == ".down") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        int nextSelection = selection + 9;
                        if (nextSelection < keys.size()) {
                            selection = nextSelection;
                        }
                        
                        renderNext();
                    }
                };
            }

            return func;
        })
    {
        if (size.w / 9 < itemSize.w) {
            itemSize.w = size.w / 9;
        }
        if (size.h / 8 < itemSize.h) {
            itemSize.h = size.h / 8;
        }
        textPos.x = itemSize.w / 2;
        textPos.y = (itemSize.h - 8) / 2;
    }

    std::vector<std::string> keys = {
        "A", "B", "C", "D", "E", "F", "G", "H", "I",
        "J", "K", "L", "M", "N", "O", "P", "Q", "R",
        "S", "T", "U", "V", "W", "X", "Y", "Z", "a",
        "b", "c", "d", "e", "f", "g", "h", "i", "j",
        "k", "l", "m", "n", "o", "p", "q", "r", "s",
        "t", "u", "v", "w", "x", "y", "z", "1", "2",
        "3", "4", "5", "6", "7", "8", "9", "0", "-",
        "=", "_", ".", "!", " "
    };

    void render()
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { bgColor });
            int y = relativePosition.y + ((size.h - (itemSize.h * 8)) * 0.5);
            int x = relativePosition.x + ((size.w - (itemSize.w * 9)) * 0.5);
            Point pos;
            for (int k = 0; k < keys.size(); k++) {
                int row = k / 9;
                int col = k % 9;
                pos = { x + col * itemSize.w, y + row * itemSize.h };
                draw.filledRect(pos, { itemSize.w - 2, itemSize.h - 2 }, { k == selection ? selectionColor : itemBackground });
                draw.textCentered({ pos.x + textPos.x, pos.y + textPos.y }, keys[k], 8, { textColor });
            }

            icon.render("&icon::backspace::filled", { pos.x + textPos.x, pos.y + textPos.y }, 6, { textColor }, Icon::CENTER);
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        if (keypadLayout.config(key, value)) {
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is the color of the background. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(value);
            return true;
        }

        /*md - `TEXT_COLOR: color` is the color of the text. */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            textColor = draw.getColor(value);
            return true;
        }

        /*md - `SELECTION_COLOR: color` is the color of the selection. */
        if (strcmp(key, "SELECTION_COLOR") == 0) {
            selectionColor = draw.getColor(value);
            return true;
        }

        /*md - `ITEM_BACKGROUND: color` is the color of the item background. */
        if (strcmp(key, "ITEM_BACKGROUND") == 0) {
            itemBackground = draw.getColor(value);
            return true;
        }

        return false;
    }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        keypadLayout.onKey(id, key, state, now);
    }
};

#endif
