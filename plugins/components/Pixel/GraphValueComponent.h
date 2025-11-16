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

    int halfHeight = 15.0f;

    int8_t extendEncoderIdArea = -1;

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

        extendEncoderIdArea = config.value("extendEncoderIdArea", extendEncoderIdArea);

        /*md md_config_end */

        resize();
    }

    void resize() override
    {
        halfHeight = size.h * 0.49;
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { bgColor });
        if (val != NULL && val->props().graph != NULL) {
            std::vector<Point> points = { { relativePosition.x, relativePosition.y + (int)(halfHeight * 0.5f) } };
            std::vector<Point> positivePoints = { { relativePosition.x, relativePosition.y + halfHeight } };

            bool onlyPositive = true;
            for (int i = 0; i < size.w; i++) {
                float index = i / (float)(size.w - 1);
                float value = CLAMP(val->props().graph(index), -1.0f, 1.0f);
                // logDebug("index: %f, value: %f", index, value);
                if (value < 0.0f) onlyPositive = false;
                float centeredValue = 1.0f - (value * 0.5f + 0.5f);
                points.push_back({ relativePosition.x + i, relativePosition.y + (int)(centeredValue * halfHeight) });
                positivePoints.push_back({ relativePosition.x + i, relativePosition.y + (int)((1.0f - value) * halfHeight) });
            }
            points.push_back({ relativePosition.x + size.w, points[0].y });
            positivePoints.push_back({ relativePosition.x + size.w, positivePoints[0].y });

            if (points.size() > 2) {
                if (filled) {
                    draw.filledPolygon(onlyPositive ? positivePoints : points, { fillColor });
                }
                if (outline) {
                    draw.lines(onlyPositive ? positivePoints : points, { outlineColor });
                }
            }
        }
    }

    const std::vector<EventInterface::EncoderPosition> getEncoderPositions() override
    {
        if (extendEncoderIdArea < 0) {
            return {};
        }

        return {
            { extendEncoderIdArea, size, relativePosition },
        };
    }
};
