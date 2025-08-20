#pragma once

#include "log.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

class WorkspaceKnobComponent : public Component {
    Color bgColor;
    Color color;
    Color labelColor;

    int fontSize = 32;
    void* font = NULL;
    int labelfontSize = 12;
    void* labelfont = NULL;

public:
    WorkspaceKnobComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , color(styles.colors.text)
        , labelColor(alpha(styles.colors.text, 0.4))
    {
        font = draw.getFont("PoppinsLight_16");
        /*md md_config:Text */
        nlohmann::json& config = props.config;

        /// The font size of the text.
        fontSize = config.value("fontSize", fontSize); //eg: 8
        /// The font of the text. Default is null.
        if (config.contains("font")) {
            font = draw.getFont(config["font"].get<std::string>().c_str()); //eg: "Sinclair_S"
        }

        /// The font size of the text.
        labelfontSize = config.value("labelFontSize", labelfontSize); //eg: 8
        /// The font of the text. Default is null.
        if (config.contains("labelFont")) {
            labelfont = draw.getFont(config["labelFont"].get<std::string>().c_str()); //eg: "Sinclair_S"
        }

        /// The background color of the text.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        /// The color of the text
        color = draw.getColor(config["color"], color); //eg: "#ffffff"

        /// The color of the text
        labelColor = draw.getColor(config["labelColor"], labelColor); //eg: "#ffffff"

        /*md md_config_end */
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { bgColor });
        int x = relativePosition.x + (size.w) * 0.5;
        int y = relativePosition.y + size.h - labelfontSize - 4;
        draw.textCentered({ x, y }, "Workspace", labelfontSize, { labelColor, .font = labelfont, .maxWidth = size.w });
        draw.textCentered({ x, y - fontSize - 10 }, "W01", fontSize, { color, .font = font, .maxWidth = size.w });
    }
};
