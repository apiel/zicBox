/** Description:
This specialized C++ header file defines an essential user interface element called the `KeyboardComponent`. It acts as a complete, interactive, on-screen keyboard designed for capturing text input, especially in environments like embedded systems or custom digital interfaces where a physical keyboard might not be present.

### Core Functionality

The main purpose of the component is to display a virtual keyboard layout, manage user selection, track the typed characters, and ultimately process the finalized input.

### Design and Interaction

The component manages the visual appearance, allowing precise control over colors for the background, text, and the currently selected key. It defines the size and position of individual keys and includes a large predefined list of characters (uppercase, lowercase, numbers, and symbols).

Interaction is handled through a sophisticated navigation system. Users can move a visible highlight (the "selection") across the grid of keys using directional commands (up, down, left, right).

### Input Processing

When a key is activated, the component performs several tasks:
1.  **Typing:** It appends the character to an internal text string, which represents the user's input.
2.  **Special Actions:** It supports dedicated actions like backspacing (deleting the last character), cancelling the input process, and navigating to a completely different screen view.
3.  **Finalization:** When the user selects "Done," the collected text string is prepared for output.

### Data Output

A key feature is its ability to communicate the collected text. The component is configured to send the final input string to an external "Audio Plugin" or data system, identified by a specific data ID, effectively completing the data entry task before navigating the user back to a specified previous screen.

sha: 555561d2be0b4ea0584f44ffa282598209f0557052b7b3b2fb88690083b11108 
*/
#pragma once

#include "plugins/components/base/Icon.h"
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

    Color bgColor;
    Color itemBackground;
    Color selectionColor;
    Color textColor;
    void* font = NULL;

    Size itemSize = { 20, 20 };
    Point textPos = { 0, 0 };

    int selection = 0;

    std::string redirectView = "";

    std::string value = "";

    AudioPlugin* plugin = NULL;
    uint8_t dataId = 0;

public:
    KeyboardComponent(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
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
            if (action == ".cancel") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        view->setView(redirectView);
                    }
                };
            }
            if (action == ".backspace")
            {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        value = value.substr(0, value.size() - 1);
                        renderNext();
                    }
                };
            }
            

            return func;
        })
        , icon(props.view->draw)
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
        , itemBackground(lighten(styles.colors.background, 0.5))
        , selectionColor(styles.colors.primary)
    {
        font = draw.getFont("PoppinsLight_8");

        if (size.w / 9 < itemSize.w) {
            itemSize.w = size.w / 9;
        }
        if (size.h / 8 < itemSize.h) {
            itemSize.h = size.h / 8;
        }
        textPos.x = itemSize.w / 2;
        textPos.y = (itemSize.h - 8) / 2;

        /*md md_config:Keyboard */
        nlohmann::json& config = props.config;

        /// The view to redirect once edit is finished.
        if (config.contains("redirectView")) {
            redirectView = config["redirectView"].get<std::string>(); //eg: "view_name"
        }

        /// The audio plugin to send the input value.
        if (config.contains("audioPlugin")) {
            plugin = &getPlugin(config["audioPlugin"].get<std::string>(), track); //eg: "audio_plugin_name"
        }

        /// The data id of the audio plugin where the input value will be sent.
        if (config.contains("dataId")) {
            dataId = plugin->getDataId(config["dataId"].get<std::string>()); //eg: "data_id"
        }

        /// Set the background color of the component.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        /// Set the color of the text.
        textColor = draw.getColor(config["textColor"], textColor); //eg: "#ffffff"

        /// Set the color of the selection.
        selectionColor = draw.getColor(config["selectionColor"], selectionColor); //eg: "#ffffff"

        /// Set the color of the item background.
        itemBackground = draw.getColor(config["itemBackground"], itemBackground); //eg: "#ffffff"

        /*md md_config_end */
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
        draw.filledRect(relativePosition, size, { bgColor });
        int y = relativePosition.y + ((size.h - (itemSize.h * 9) + 10) * 0.5);
        int x = relativePosition.x + ((size.w - (itemSize.w * 9)) * 0.5);

        draw.filledRect({ x, y }, { itemSize.w * 9, itemSize.h }, { itemBackground });
        draw.text({ x + 8, y + textPos.y }, value, 8, { textColor, .font = font });

        y += itemSize.h + 10;

        Point pos;
        for (int k = 0; k < keys.size(); k++) {
            int row = k / 9;
            int col = k % 9;
            pos = { x + col * itemSize.w, y + row * itemSize.h };
            draw.filledRect(pos, { itemSize.w - 2, itemSize.h - 2 }, { k == selection ? selectionColor : itemBackground });
            Point posText = { pos.x + textPos.x, pos.y + textPos.y };
            if (!icon.render(keys[k], { posText.x + itemSize.w / 2, posText.y }, 6, { textColor })) {
                draw.textCentered(posText, keys[k], 8, { textColor, .font = font });
            }
        }
        draw.filledRect({ pos.x + itemSize.w, pos.y }, { (itemSize.w * 2) - 2, itemSize.h - 2 }, { keys.size() == selection ? selectionColor : itemBackground });
        draw.textCentered({ pos.x + itemSize.w + ((itemSize.w * 2) - 2) / 2, pos.y + textPos.y }, "Back", 8, { textColor, .font = font });

        draw.filledRect({ pos.x + itemSize.w * 3, pos.y }, { (itemSize.w * 2) - 2, itemSize.h - 2 }, { keys.size() + 1 == selection ? selectionColor : itemBackground });
        draw.textCentered({ pos.x + itemSize.w * 3 + ((itemSize.w * 2) - 2) / 2, pos.y + textPos.y }, "Done", 8, { textColor, .font = font });
    }
};
