/** Description:
This file defines a reusable software module, or "component," specifically designed for graphical applications, called `RectComponent`. Its sole purpose is to efficiently draw a rectangular shape on the screen or user interface.

This component functions as a customizable building block, inheriting standard features from a broader system framework.

### Functionality and Customization

The core function of this component is defined by two configurable properties:

1.  **Color:** Specifies the visual shade of the rectangle.
2.  **Fill Status:** Determines the style of the rectangle. It can be set to draw a completely solid, filled shape, or merely an outline (border). By default, the rectangle is drawn as a solid shape.

When the component is first created, it automatically reads configuration settings—such as specific color codes or the desired fill status—to initialize itself.

### How It Works

When the system requests the component to display itself (via its `render` function), it performs a quick check on the "Fill Status."

*   If the shape is configured to be **filled**, it calls the system's drawing tool to render a solid rectangle using the specified color.
*   If the shape is configured as an **outline**, it calls a different drawing tool to render only the border of the rectangle.

This structure allows developers to easily integrate highly customizable rectangles into any layout without needing to rewrite the drawing logic every time.

sha: e9e65b6f74ffc283fe89500243b13b7b0dc003d6ec7f5119cafdce6ac2b53b26 
*/
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
