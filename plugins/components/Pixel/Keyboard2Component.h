/** Description:
This code defines the blueprint for a specialized user interface element called the `Keyboard2Component`. Its primary purpose is to display a customizable, on-screen keyboard, typically within a larger graphical or audio application environment.

This keyboard component is highly modular and customizable through its settings:
1.  **Visuals:** It allows defining colors for the background, text, and individual keys, as well as selecting the font and font size for labels and the input area.
2.  **Functionality:** It manages two sets of key labels (standard and "shifted"—like lowercase and uppercase), which it can switch between based on an external signal (the "shift context").

The component’s internal logic manages user interactions:
*   Typing actions append characters to the stored input text (`value`).
*   Special actions like "backspace" delete the last character.
*   "Exit" clears the current input and navigates the user back to a predetermined screen (`redirectView`).

Crucially, when the user finalizes the input (often via an "OK" action), the component sends the accumulated text string to a specific destination: an `AudioPlugin`. This allows the typed information to control parameters or settings within the audio processing system before the component switches the display back to the main view.

In summary, this code creates an interactive, visually adaptable keyboard that captures user input and links it directly to core functionality within the surrounding application framework.

sha: 63a8fb6b601a8e3a37a3f4b0bd04969571f50fb3d357d43de3c4b71874fe699b 
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

        resize();
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
            if (!icon.render(currentKeys[k], posText, fontSize, { textColor }, Icon::CENTER)) {
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
