/** Description:
This code defines a specialized software building block, known as a **Text Component**, designed for displaying highly customizable text within a graphical application or user interface.

**Basic Functionality**

The component acts like a smart label or text box. Its main role is to manage and display a specific string of text on the screen, adhering to various visual rules. It integrates with the system's drawing engine to handle graphics and typography.

**How It Works**

When the component is created, it reads its settings from a configuration source. These settings determine its appearance and behavior:

1.  **Content:** It stores the actual words or phrase to be shown.
2.  **Appearance:** It sets the colors for the text itself and its background area.
3.  **Typography:** It allows choosing a specific font, setting the font size, and adjusting the height of the characters.
4.  **Alignment:** The component calculates where to draw the text based on alignment instructions—it can display the text centered, aligned to the right, or aligned to the left.

During the display process (called "rendering"), the component first draws a solid background color in its designated area. Then, it uses the stored alignment settings to accurately position and draw the text string using the specified font and colors. It can also handle the rendering of simple icons alongside or instead of standard text.

sha: 9def5e233596ad568ccb7959c741c23b2427eb08177f8f6fcaf8337b5437d3a1 
*/
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
    bool rightAligned = false;
    int fontSize = 12;
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
        nlohmann::json& config = props.config;
        /// The text to display.
        text = getConfig(config, "text"); //eg: "Hello World"
        /// If true, the text will be centered. Default is false.
        centered = config.value("centered", centered); //eg: true
        /// If true, the text will be right aligned. Default is false.
        rightAligned = config.value("right", rightAligned); //eg: true

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

        /*md md_config_end */
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { bgColor });
        if (!text.empty()) {
            if (rightAligned) {
                Point textPos = { relativePosition.x + size.w, relativePosition.y };
                if (!icon.render(text, textPos, 8, { color })) {
                    draw.textRight(textPos, text, fontSize, { color, .font = font, .maxWidth = size.w, .fontHeight = fontHeight });
                }
            } else if (centered) {
                Point textPos = { relativePosition.x + (int)(size.w * 0.5), relativePosition.y + (int)((size.h - fontSize) * 0.5) };
                if (!icon.render(text, textPos, 8, { color })) {

                    draw.textCentered(textPos, text, fontSize, { color, .font = font, .maxWidth = size.w, .fontHeight = fontHeight });
                }
            } else {
                if (!icon.render(text, { relativePosition.x, relativePosition.y }, fontSize, { color })) {
                    draw.text({ relativePosition.x, relativePosition.y }, text, fontSize, { color, .font = font, .maxWidth = size.w, .fontHeight = fontHeight });
                }
            }
        }
    }
};
