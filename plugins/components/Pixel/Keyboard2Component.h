/** Description:
This header file defines the **Keyboard2Component**, a sophisticated virtual keyboard module designed for use within a larger graphical or audio application framework. It functions as a flexible input device, allowing users to type text directly into the system.

### Purpose and Operation

The core function of this component is to manage and capture user text input. When the user interacts with the displayed keys, the component collects those characters and stores them internally as a continuous text string.

The component is highly interactive, managing specialized actions such as:
1.  **Typing:** Appending characters to the input value.
2.  **Backspace:** Deleting the last character entered.
3.  **OK/Done:** Finalizing the input and saving the result.
4.  **Exit:** Discarding the current input and navigating away.

### Customization and Integration

The appearance is fully customizable, allowing settings for background color, text color, and the specific font used for labels and input text.

Crucially, the keyboard is designed to integrate with an external system, often an **Audio Plugin**. During setup, it is configured to link to a specific data location (a `dataId`) within that plugin. Once the user presses "OK," the component sends the final entered text string directly to that designated data slot, allowing the text to control plugin settings or parameters.

### Dynamic Features

The keyboard supports dynamic key sets, similar to a physical keyboard's **Shift** function. By checking a shared system setting (a *context variable*), the component can instantly switch between displaying the standard key labels (e.g., lowercase) and an alternative set (e.g., uppercase or symbols).

The layout is fully responsive; the component automatically calculates the necessary size for each individual key based on the overall space it occupies on the screen, ensuring a readable and usable interface regardless of display size. Once the input is confirmed, the component automatically redirects the user to a predefined screen.

sha: 421482a20830a70c582c308aec71af9be28eb1dcff00a81b234095058ce72336 
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

class Keyboard2Component : public Component {
protected:
    Icon icon;

    Color bgColor;
    Color itemBackground;
    Color textColor;
    void* font = NULL;
    int fontSize = 8;

    Size itemSize = { 0, 0 };
    Point textPos = { 0, 0 };

    int selection = 0;

    std::string redirectView = "";

    std::string value = "";

    AudioPlugin* plugin = NULL;
    uint8_t dataId = 0;

    uint8_t shiftContextId = 0;

    std::vector<std::string> keys;
    std::vector<std::string> keysShifted;
    std::vector<Color> keyColors;

public:
    Keyboard2Component(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;

            if (action.rfind(".type:") == 0) {
                func = [this, action](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        std::string key = action.substr(6);
                        value += key;
                        renderNext();
                    }
                };
            }
            if (action == ".ok") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        done();
                    }
                };
            }
            if (action == ".exit") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        value = "";
                        view->setView(redirectView);
                    }
                };
            }
            if (action == ".backspace") {
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
    {
        font = draw.getFont("PoppinsLight_8");

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

        /// Set the color of the item background.
        itemBackground = draw.getColor(config["itemBackground"], itemBackground); //eg: "#ffffff"

        /// Set context id shared to define the shift.
        shiftContextId = config.value("shiftContextId", shiftContextId); //eg: 10

        /// The font
        if (config.contains("font")) {
            font = draw.getFont(config["font"].get<std::string>().c_str()); //eg: "PoppinsLight_8"
            fontSize = draw.getDefaultFontSize(font);
        }

        /// Keys labels
        if (config.contains("keyLabels") && config["keyLabels"].is_array()) {
            keys = config["keyLabels"].get<std::vector<std::string>>(); //eg: ["a", "b", "c", ...]
        }

        /// Keys labels shifted
        if (config.contains("keyLabelsShifted") && config["keyLabelsShifted"].is_array()) {
            keysShifted = config["keyLabelsShifted"].get<std::vector<std::string>>(); //eg: ["A", "B", "C", ...]
        }

        // Keys colors
        if (config.contains("keyColors") && config["keyColors"].is_array()) {
            for (int i = 0; i < config["keyColors"].size(); i++) {
                keyColors.push_back(draw.getColor(config["keyColors"][i].get<std::string>()));
            }
        }

        /*md md_config_end */

        // resize();
    }

    void resize() override
    {
        itemSize.w = size.w / 10;
        itemSize.h = (size.h-5) / 5;
        textPos.x = itemSize.w / 2;
        textPos.y = (itemSize.h - fontSize) / 2;
    }

    void done()
    {
        if (plugin != NULL && !value.empty()) {
            plugin->data(dataId, &value);
            view->setView(redirectView);
        }
    }

    void render()
    {
        draw.filledRect(relativePosition, size, { bgColor });
        int y = relativePosition.y;
        int x = relativePosition.x;

        draw.filledRect({ x, y }, { size.w, itemSize.h }, { itemBackground });
        draw.text({ x + 8, y + textPos.y }, value, fontSize, { textColor, .font = font });

        y += itemSize.h + 5;

        Point pos;
        std::vector<std::string>& currentKeys = view->contextVar[shiftContextId] != 0 ? keysShifted : keys;
        for (int k = 0; k < currentKeys.size(); k++) {
            int row = k / 10;
            int col = k % 10;
            pos = { x + col * itemSize.w, y + row * itemSize.h };
            Color color = k < keyColors.size() ? keyColors[k] : itemBackground;
            draw.filledRect(pos, { itemSize.w - 2, itemSize.h - 2 }, { color });
            Point posText = { pos.x + textPos.x, pos.y + textPos.y };
            if (!icon.render(currentKeys[k], { posText.x + itemSize.w / 2, posText.y }, fontSize, { textColor })) {
                draw.textCentered(posText, currentKeys[k], fontSize, { textColor, .font = font });
            }
        }
    }

    void onContext(uint8_t index, float value) override
    {
        if (index == shiftContextId) {
            renderNext();
        }
        Component::onContext(index, value);
    }
};
