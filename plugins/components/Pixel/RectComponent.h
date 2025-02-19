#pragma once

#include "plugins/components/component.h"

#include <string>

/*md
## Rect

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/rect.png" />

Rect components to draw a rectangle.
*/

class RectComponent : public Component {
protected:
    Color color;

    bool filled = true;

public:
    RectComponent(ComponentInterface::Props props)
        : Component(props)
        , color(styles.colors.background)
    {
        /*md md_config:Rect */
        nlohmann::json& config = props.config;

        /// The color of the rectangle.
        color = draw.getColor(config["color"], color); //eg: "#000000"

        /// If true, the rectangle will be filled. Default is true.
        filled = config.value("filled", filled); //eg: false

        /*md md_config_end */
    }
    void render()
    {
        if (filled) {
            draw.filledRect(relativePosition, size, { color });
        } else {
            draw.rect(relativePosition, size, { color });
        }
    }
};
