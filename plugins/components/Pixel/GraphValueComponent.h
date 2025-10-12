#pragma once

#include "helpers/clamp.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <stdexcept>

/*md
## GraphValueComponent
*/

class GraphValueComponent : public Component {
    ValueInterface* val = NULL;

    bool inverted = false;

    bool filled = true;
    bool outline = true;

    Color bgColor;

    Color fillColor;
    Color outlineColor;

    float halfHeight = 15.0f;

public:
    GraphValueComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , fillColor(styles.colors.primary)
        , outlineColor(lighten(styles.colors.primary, 0.5))
    {

        /*md md_config:GraphEncoder */
        nlohmann::json& config = props.config;

        /// The audio plugin to get control on.
        AudioPlugin* audioPlugin = getPluginPtr(config, "audioPlugin", track); //eg: "audio_plugin_name"

        /// The audio plugin key parameter to get control on.
        std::string param = getConfig(config, "param"); //eg: "parameter_name"
        val = watch(audioPlugin->getValue(param));

        /// If true, the data is inverted. Default is false.
        inverted = config.value("inverted", inverted);

        outline = config.value("outline", outline);
        filled = config.value("filled", filled);
        bgColor = draw.getColor(config["bgColor"], bgColor);
        fillColor = draw.getColor(config["fillColor"], fillColor);
        outlineColor = draw.getColor(config["outlineColor"], outlineColor);

        /*md md_config_end */

        if (val != NULL && val->props().graph == NULL) { // if graph is not available, set val to null
            val = NULL;
        }

        resize();
    }

    void resize() override
    {
        halfHeight = size.h * 0.49;
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { bgColor });
        std::vector<Point> relativePoints = getPoints();

        if (relativePoints.size() > 2) {
            for (auto& point : relativePoints) {
                point.y += relativePosition.y;
                point.x += relativePosition.x;
            }
            if (filled) {
                draw.filledPolygon(relativePoints, { fillColor });
            }
            if (outline) {
                draw.lines(relativePoints, { outlineColor });
            }
        }
    }

    std::vector<Point> getPoints()
    {
        std::vector<Point> points = {};
        if (val != NULL) {
            points.push_back({ 0, (int)(halfHeight) });

            for (int i = 0; i < size.w; i++) {
                float index = i / (float)(size.w - 1);
                float value = val->props().graph(index);
                points.push_back({ i, (int)(CLAMP(value, -1.0f, 1.0f) * halfHeight + halfHeight) });
            }
            
            points.push_back({ size.w, (int)(halfHeight) });

            if (inverted) {
                for (auto& point : points) {
                    point.y = size.h - point.y;
                }
            }
        }
        return points;
    }
};
