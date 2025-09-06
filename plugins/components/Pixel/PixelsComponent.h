#pragma once

#include "plugins/components/component.h"

#include <string>

class PixelsComponent : public Component {
protected:
    Color color;

    int count = 100;

public:
    PixelsComponent(ComponentInterface::Props props)
        : Component(props)
        , color(styles.colors.white)
    {
        /*md md_config:Rect */
        nlohmann::json& config = props.config;

        /// The color of the rectangle.
        color = draw.getColor(config["color"], color); //eg: "#000000"

        /// The number of pixels to draw.
        count = config.value("count", count);

        /*md md_config_end */
    }
    void render()
    {
        for (int i = 0; i < count; i++) {
            draw.pixel({ relativePosition.x + rand() % size.w, relativePosition.y + rand() % size.h }, { color });
        }
    }
};
