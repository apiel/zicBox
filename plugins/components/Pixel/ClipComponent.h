#pragma once

#include "plugins/components/component.h"

#include <string>

/*md
## Clip
*/

class ClipComponent : public Component {
protected:
    Color bgColor;
    Color textColor;

    int fontSize = 12;
    void* font = NULL;

public:
    ClipComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
    {
        /*md md_config:Rect */
        nlohmann::json& config = props.config;

        /// The background color.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        /// The color of the text
        textColor = draw.getColor(config["color"], textColor); //eg: "#ffffff"

        /// The font of the text. Default is null.
        if (config.contains("font")) {
            font = draw.getFont(config["font"].get<std::string>().c_str()); //eg: "Sinclair_S"
            fontSize = draw.getDefaultFontSize(font);
        }
        /// The font size of the text.
        fontSize = config.value("fontSize", fontSize); //eg: 8

        /*md md_config_end */
    }
    void render()
    {
        draw.filledRect(relativePosition, size, { bgColor });

        draw.text({ relativePosition.x, relativePosition.y }, "Clip", fontSize, { textColor, .font = font });
    }
};
