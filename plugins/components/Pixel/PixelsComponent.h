/** Description:
This code defines a specialized reusable graphical element known as the `PixelsComponent`. It functions as a building block, likely within a plugin system or a framework designed for rendering visualizations and graphical user interfaces.

**What it Does**
The primary function of this component is to draw a specific quantity of colored dots (pixels) scattered randomly within its assigned area on the screen.

**How it is Configured**
When the `PixelsComponent` is first created, it processes configuration settings provided to it. This allows users or developers to customize its appearance without altering the source code:
1.  **Color:** Users can define the specific color for the dots, which overrides the default color (initially set to white).
2.  **Count:** Users can specify the total number of pixels the component should draw. If this setting is not provided, the component defaults to drawing 100 pixels.

**The Rendering Process**
When the component is instructed to draw itself, it loops based on the configured count. In each loop, it calculates a random position within its designated screen boundaries and then places a single colored pixel at that exact, randomized location, creating a visual scattering effect.

sha: f2d12f92a9626c903494c4ca4a631e4ca91b2cb3b1f25f96577a82c213ea518e 
*/
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
