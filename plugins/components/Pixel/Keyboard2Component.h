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
    Color selectionColor;
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

public:
    Keyboard2Component(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;

            return func;
        })
        , icon(props.view->draw)
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
        , itemBackground(lighten(styles.colors.background, 0.5))
        , selectionColor(styles.colors.primary)
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

        /// Set the color of the selection.
        selectionColor = draw.getColor(config["selectionColor"], selectionColor); //eg: "#ffffff"

        /// Set the color of the item background.
        itemBackground = draw.getColor(config["itemBackground"], itemBackground); //eg: "#ffffff"

        /// The font
        if (config.contains("font")) {
            font = draw.getFont(config["font"].get<std::string>().c_str()); //eg: "PoppinsLight_8"
            fontSize = draw.getDefaultFontSize(font);
        }

        /*md md_config_end */

        resize();
    }

    void resize() override
    {
        itemSize.w = size.w / 10;
        itemSize.h = size.h / 8;
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
        "A",
        "B",
        "C",
        "D",
        "E",
        "F",
        "G",
        "H",
        "I",
        "J",
        "K",
        "L",
        "M",
        "N",
        "O",
        "P",
        "Q",
        "R",
        "S",
        "T",
        "U",
        "V",
        "W",
        "X",
        "Y",
        "Z",
        "a",
        "b",
        "c",
        "d",
        "e",
        "f",
        "g",
        "h",
        "i",
        "j",
        "k",
        "l",
        "m",
        "n",
        "o",
        "p",
        "q",
        "r",
        "s",
        "t",
        "u",
        "v",
        "w",
        "x",
        "y",
        "z",
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",
        "0",
        "-",
        "=",
        "_",
        ".",
        "!",
        "%",
        "&",
        ":",
        "$",
        "#",
    };

    void render()
    {
        draw.filledRect(relativePosition, size, { bgColor });
        int y = relativePosition.y;
        int x = relativePosition.x;

        draw.filledRect({ x, y }, { size.w, itemSize.h }, { itemBackground });
        draw.text({ x + 8, y + textPos.y }, value, fontSize, { textColor, .font = font });

        y += itemSize.h + 10;

        Point pos;
        for (int k = 0; k < keys.size(); k++) {
            int row = k / 9;
            int col = k % 9;
            pos = { x + col * itemSize.w, y + row * itemSize.h };
            draw.filledRect(pos, { itemSize.w - 2, itemSize.h - 2 }, { k == selection ? selectionColor : itemBackground });
            Point posText = { pos.x + textPos.x, pos.y + textPos.y };
            // if (!icon.render(keys[k], posText, 6, { textColor }, Icon::CENTER)) {
            draw.textCentered(posText, keys[k], fontSize, { textColor, .font = font });
            // }
        }
        // draw.filledRect({ pos.x + itemSize.w, pos.y }, { (itemSize.w * 2) - 2, itemSize.h - 2 }, { keys.size() == selection ? selectionColor : itemBackground });
        // draw.textCentered({ pos.x + itemSize.w + ((itemSize.w * 2) - 2) / 2, pos.y + textPos.y }, "Back", fontSize, { textColor, .font = font });

        // draw.filledRect({ pos.x + itemSize.w * 3, pos.y }, { (itemSize.w * 2) - 2, itemSize.h - 2 }, { keys.size() + 1 == selection ? selectionColor : itemBackground });
        // draw.textCentered({ pos.x + itemSize.w * 3 + ((itemSize.w * 2) - 2) / 2, pos.y + textPos.y }, "Done", fontSize, { textColor, .font = font });

        pos = { x + 9 * itemSize.w, y };
        draw.filledRect(pos, { itemSize.w - 2, itemSize.h - 2 }, { itemBackground });
        icon.render("&icon::backspace::filled", { pos.x + textPos.x, pos.y + textPos.y }, fontSize, { textColor }, Icon::CENTER);

        pos.y += itemSize.h;
        draw.filledRect(pos, { itemSize.w - 2, itemSize.h - 2 }, { itemBackground });
        draw.textCentered({ pos.x + textPos.x, pos.y + textPos.y }, "Ok", fontSize, { textColor, .font = font });

        pos.y += itemSize.h;
        draw.filledRect(pos, { itemSize.w - 2, itemSize.h - 2 }, { itemBackground });
        draw.textCentered({ pos.x + textPos.x, pos.y + textPos.y }, "Exit", fontSize, { textColor, .font = font });

        pos.y += itemSize.h;
        draw.filledRect(pos, { itemSize.w - 2, itemSize.h - 2 }, { itemBackground });
        draw.textCentered({ pos.x + textPos.x, pos.y + textPos.y }, "Shift", fontSize, { textColor, .font = font });
    }
};
