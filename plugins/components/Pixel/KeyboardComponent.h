#ifndef _UI_PIXEL_COMPONENT_KEYBOARD_H_
#define _UI_PIXEL_COMPONENT_KEYBOARD_H_

#include "plugins/components/base/Icon.h"
#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/VisibilityContext.h"
#include "plugins/components/utils/VisibilityGroup.h"
#include "plugins/components/utils/color.h"

#include <string>

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

    std::string redirectView = "";

    std::string value = "";

    AudioPlugin* plugin = NULL;
    uint8_t dataId = 0;

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
                            selection = keys.size() + 1;
                        }
                        renderNext();
                    }
                };
            }
            if (action == ".right") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        selection++;
                        if (selection > keys.size() + 1) {
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
                        if (nextSelection <= keys.size() + 1) {
                            selection = nextSelection;
                        } else {
                            selection = keys.size() + 1;
                        }

                        renderNext();
                    }
                };
            }
            if (action == ".type") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (selection == keys.size() + 1) {
                            done();
                        } else if (selection == keys.size()) {
                            view->setView(redirectView);
                        } else if (selection == keys.size() - 1) {
                            value = value.substr(0, value.size() - 1);
                            // printf("%s\n", value.c_str());
                        } else if (value.size() < 21) {
                            value += keys[selection];
                            // printf("%s\n", value.c_str());
                        }
                        renderNext();
                    }
                };
            }
            if (action == ".done") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        done();
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

    void done()
    {
        if (plugin != NULL && !value.empty()) {
            plugin->data(dataId, &value);
            view->setView(redirectView);
        }
    }

    std::vector<std::string> keys = {
        "A", "B", "C", "D", "E", "F", "G", "H", "I",
        "J", "K", "L", "M", "N", "O", "P", "Q", "R",
        "S", "T", "U", "V", "W", "X", "Y", "Z", "a",
        "b", "c", "d", "e", "f", "g", "h", "i", "j",
        "k", "l", "m", "n", "o", "p", "q", "r", "s",
        "t", "u", "v", "w", "x", "y", "z", "1", "2",
        "3", "4", "5", "6", "7", "8", "9", "0", "-",
        "=", "_", ".", "!", "&icon::backspace::filled"
    };

    void render()
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { bgColor });
            int y = relativePosition.y + ((size.h - (itemSize.h * 9) + 10) * 0.5);
            int x = relativePosition.x + ((size.w - (itemSize.w * 9)) * 0.5);

            draw.filledRect({ x, y }, { itemSize.w * 9, itemSize.h }, { itemBackground });
            draw.text({ x + 8, y + textPos.y }, value, 8, { textColor });

            y += itemSize.h + 10;

            Point pos;
            for (int k = 0; k < keys.size(); k++) {
                int row = k / 9;
                int col = k % 9;
                pos = { x + col * itemSize.w, y + row * itemSize.h };
                draw.filledRect(pos, { itemSize.w - 2, itemSize.h - 2 }, { k == selection ? selectionColor : itemBackground });
                Point posText = { pos.x + textPos.x, pos.y + textPos.y };
                if (!icon.render(keys[k], posText, 6, { textColor }, Icon::CENTER)) {
                    draw.textCentered(posText, keys[k], 8, { textColor });
                }
            }
            draw.filledRect({ pos.x + itemSize.w, pos.y }, { (itemSize.w * 2) - 2, itemSize.h - 2 }, { keys.size() == selection ? selectionColor : itemBackground });
            draw.textCentered({ pos.x + itemSize.w + ((itemSize.w * 2) - 2) / 2, pos.y + textPos.y }, "Back", 8, { textColor });

            draw.filledRect({ pos.x + itemSize.w * 3, pos.y }, { (itemSize.w * 2) - 2, itemSize.h - 2 }, { keys.size() + 1 == selection ? selectionColor : itemBackground });
            draw.textCentered({ pos.x + itemSize.w * 3 + ((itemSize.w * 2) - 2) / 2, pos.y + textPos.y }, "Done", 8, { textColor });
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        if (keypadLayout.config(key, value)) {
            return true;
        }

        /*md - `REDIRECT_VIEW: viewName` is the view to return when the edit is finished. */
        if (strcmp(key, "REDIRECT_VIEW") == 0) {
            redirectView = value;
            return true;
        }

        /*md - `DONE_DATA: plugin data_id` is the data id to return when the edit is done. */
        if (strcmp(key, "DONE_DATA") == 0) {
            plugin = &getPlugin(strtok(value, " "), track);
            dataId = plugin->getDataId(strtok(NULL, " "));
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
