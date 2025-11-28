/** Description:
This component, named `GraphValueComponent`, is a specialized visual element designed to display real-time control data, typically from an audio processing or plugin system. Think of it as a small, dynamic visualization widget.

**Core Functionality:**
The primary role of this component is to monitor a specific numerical parameter within an audio plugin (like a volume level, filter cutoff, or modulation amount) and draw its recent activity as a graph or waveform. This provides immediate visual feedback on the state of the controlled setting.

**How It Works:**
1.  **Configuration:** When initialized, the component connects itself to a designated parameter using its name and the specific audio plugin. It also reads settings to customize its appearance, such as background, fill, and outline colors, and whether the data should be displayed inverted.
2.  **Visualization:** During the rendering process, the component retrieves the latest sequence of data points from the parameter it is watching. It then plots these points across its display area, transforming the raw numbers into a graphical shape.
3.  **Display Style:** The graph can be drawn in various styles—as a simple outline, or as a solid shape (filled) bounded by the data points. The component automatically adjusts the height of the displayed graph when the size of the widget changes.
4.  **Interaction:** It also includes a mechanism to define a physical touch area associated with the graph, allowing external hardware controls, like rotary encoders or knobs, to interact directly with the displayed data source.

In essence, the `GraphValueComponent` acts as a crucial monitor, translating abstract control values into an easy-to-understand, customizable visual display.

sha: a99a36f5da711650010bb3f0c027fa44ac32a33738686f1a0753630e919296c1 
*/
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

        // resize();
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
