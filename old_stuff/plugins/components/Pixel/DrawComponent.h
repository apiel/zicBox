#pragma once

#include "plugins/components/component.h"

#include <string>

/*md
## Draw

Draw primitives on the view, like lines, ...
*/

class DrawComponent : public Component {
protected:
    Color color;
    Color bgColor;
    std::vector<Point> lines;
    int thickness = 1;

public:
    DrawComponent(ComponentInterface::Props props)
        : Component(props)
        , color(styles.colors.primary)
        , bgColor(styles.colors.background)
    {
        /*md md_config:Lines */
        nlohmann::json& config = props.config;

        /// The color of the rectangle.
        color = draw.getColor(config["color"], color); //eg: "#000000"

        /// The number of pixels to draw.
        thickness = config.value("thickness", thickness);

        if (config.contains("lines") && config["lines"].is_array()) {
            for (auto& point : config["lines"]) {
                if (!point.is_array() || point.size() != 2 || !point[0].is_number() || !point[1].is_number()) {
                    continue;
                }
                lines.push_back({ point[0].get<int>() + relativePosition.x, point[1].get<int>() + relativePosition.y });
            }
            if (lines.size() < 2) {
                logWarn("Lines: must have at least 2 points");
            }
        }

        /*md md_config_end */
    }
    void render()
    {
        draw.filledRect(relativePosition, size, { bgColor });

        if (lines.size() >= 2) {
            draw.lines(lines, { color, .thickness = thickness });
        }

        // TODO implement other primitives, cicle, arc, filledPolygon
    }
};
