#pragma once

#include "log.h"
#include "plugins/components/base/Icon.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

/*md
## TextBox

Multi lines text.
*/

class TextBoxComponent : public Component {
protected:
    Color bgColor;
    Color color;
    Color color2;

    std::string text;

    int fontSize = 12;
    int fontHeight = 0;
    void* font = NULL;

public:
    TextBoxComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , color(styles.colors.text)
        , color2(styles.colors.white)
    {
        /*md md_config:Text */
        nlohmann::json& config = props.config;
        /// The text to display.
        text = getConfig(config, "text"); //eg: "Hello World"

        /// The font of the text. Default is null.
        if (config.contains("font")) {
            font = draw.getFont(config["font"].get<std::string>().c_str()); //eg: "Sinclair_S"
            fontSize = draw.getDefaultFontSize(font);
        }
        /// The font size of the text.
        fontSize = config.value("fontSize", fontSize); //eg: 8

        /// The font height of the text. Default is 0.
        fontHeight = config.value("fontHeight", fontHeight); //eg: 0
        /// The background color of the text.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        /// The color of the text
        color = draw.getColor(config["color"], color); //eg: "#ffffff"

        /// The color of the text
        color2 = draw.getColor(config["color2"], color2); //eg: "#ffffff"

        /*md md_config_end */
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { bgColor });
        draw.textBox(relativePosition, size, text, fontSize, { color, .font = font, .fontHeight = fontHeight, .color2 = color2 });
    }
};
