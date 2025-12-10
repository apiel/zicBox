#pragma once

#include "log.h"
#include "plugins/components/base/Icon.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

/*md
## Icon

Icon component is used to display icons.
*/

class IconComponent : public Component {
    Icon icon;

    Color bgColor;
    Color color;

    std::string name;

    bool centered = false;
    bool rightAligned = false;
    int iconSize = 0;

public:
    IconComponent(ComponentInterface::Props props)
        : Component(props)
        , icon(props.view->draw)
        , bgColor(styles.colors.background)
        , color(styles.colors.text)
    {
        /*md md_config:Icon */
        nlohmann::json& config = props.config;
        /// The icon to display.
        name = getConfig(config, "name"); //eg: "&icon::musicNote"

        /// If true, the text will be centered. Default is false.
        centered = config.value("centered", centered); //eg: true

        /// If true, the text will be right aligned. Default is false.
        rightAligned = config.value("right", rightAligned); //eg: true

        /// The background color of the text.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        /// The color of the text
        color = draw.getColor(config["color"], color); //eg: "#ffffff"

        /*md md_config_end */
    }

    void resize() override
    {
        iconSize = size.h > size.w ? size.w : size.h;
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { bgColor });
        if (!name.empty()) {
            if (rightAligned) {
                icon.render(name, { relativePosition.x + size.w, relativePosition.y }, iconSize, { color }, Icon::RIGHT);
            } else if (centered) {
                icon.render(name, { relativePosition.x + (int)(size.w * 0.5), relativePosition.y }, iconSize, { color }, Icon::CENTER);
            } else {
                icon.render(name, { relativePosition.x, relativePosition.y }, iconSize, { color }, Icon::LEFT);
            }
        }
    }
};
