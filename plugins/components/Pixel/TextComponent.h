#pragma once

#include "log.h"
#include "plugins/components/base/Icon.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

/*md
## Text

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/text.png" />

Text component is used to display text.
*/

class TextComponent : public Component {
    Icon icon;

    Color bgColor;
    Color color;

    std::string text;

    bool centered = false;
    int fontSize = 8;
    int fontHeight = 0;
    void* font = NULL;

public:
    TextComponent(ComponentInterface::Props props)
        : Component(props)
        , icon(props.view->draw)
        , bgColor(styles.colors.background)
        , color(styles.colors.text)
    {
        /*md md_config:Text */
        nlohmann::json config = props.config;
        /// The text to display.
        text = getConfig(config, "text"); //eg: "Hello World"
        /// If true, the text will be centered. Default is false.
        centered = config.value("centered", centered); //eg: true
        /// The font size of the text. Default is 8.
        fontSize = config.value("fontSize", 8); //eg: 8
        /// The font of the text. Default is null.
        if (config.contains("font")) {
            font = draw.getFont(config["font"].get<std::string>().c_str()); //eg: "Sinclair_S"
        }
        /// The font height of the text. Default is 0.
        fontHeight = config.value("fontHeight", fontHeight); //eg: 0
        /// The background color of the text.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        /// The color of the text
        color = draw.getColor(config["color"], color); //eg: "#ffffff"

        /*md md_config_end */
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { bgColor });
        if (!text.empty()) {
            if (centered) {
                Point textPos = { relativePosition.x + (int)(size.w * 0.5), relativePosition.y + (int)(size.h * 0.5) - 4 };
                if (!icon.render(text, textPos, 8, { color }, Icon::CENTER)) {

                    draw.textCentered(textPos, text, fontSize, { color, .font = font, .maxWidth = size.w, .fontHeight = fontHeight });
                }
            } else {
                if (!icon.render(text, { relativePosition.x, relativePosition.y }, fontSize, { color }, Icon::LEFT)) {
                    draw.text({ relativePosition.x, relativePosition.y }, text, fontSize, { color, .font = font, .maxWidth = size.w, .fontHeight = fontHeight });
                }
            }
        }
    }

    bool isActive = true;
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
};
