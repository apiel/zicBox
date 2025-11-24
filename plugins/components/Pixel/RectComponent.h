/** Description:
This file defines a core graphical element called the `RectComponent`. This component acts as a specialized drawing tool within a larger application framework, specifically designed to handle the creation and rendering of rectangles on a display.

**Basic Idea and Functionality**

The `RectComponent` extends a base component, giving it inherent abilities to manage its position, size, and integration into the rendering queue. Its primary function is handled by its `render` capability.

**Configuration and Customization**

When the component is first created, it processes configuration data (often read from a setup file) to determine how the rectangle should look. Users can primarily control two aspects:

1.  **Color:** Defining the hue of the rectangle.
2.  **Fill Status:** Specifying whether the rectangle should be drawn as a completely solid, filled shape (the default behavior) or simply as a hollow outline border.

**How It Works**

When the system calls the component to draw itself, it checks its internal "filled" setting. If it is set to filled, it instructs the underlying graphics library to draw a solid shape using the stored color. If the setting indicates it should not be filled, it instructs the library to draw only the perimeter, effectively creating a box outline. This separation allows the component to efficiently manage the display of customizable geometric shapes.

sha: 0b149db735d8fe2a2c1b52556e7391246a22bff5d62b4132faa19fc8f6cc16d9 
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
