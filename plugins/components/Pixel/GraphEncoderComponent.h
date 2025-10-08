#pragma once

#include "utils/BaseGraphComponent.h"
#include <stdexcept>

/*md
## GraphEncoderComponent

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/graphEncoder.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/graphEncoder2.png" />

Show a representation of data points and provide a way to control them.

*/

class GraphEncoderComponent : public BaseGraphComponent {
    uint8_t dataId = -1;

    bool isArray = false;
    bool inverted = false;

public:
    GraphEncoderComponent(ComponentInterface::Props props)
        : BaseGraphComponent(props)
    {
        /*md md_config:GraphEncoder */
        nlohmann::json& config = props.config;

        /// If true, the rectangle will be outlined. Default is true.
        //md   outline={false}

        /// Set the color of the graph.
        //md   fillColor="#000000"

        /// If true, the rectangle will be filled. Default is true.
        //md   filled={false}

        /// Set the color of the graph outline.
        //md   outlineColor="#000000"

        /// Set the color of the background.
        //md   bgColor="#000000"

        /// The audio plugin to get control on.
        //md   plugin="audio_plugin_name"

        if (plugin == NULL) {
            logWarn("GraphEncoderComponent cannot init: plugin is not set");
            return;
        }

        /// The data id of the audio plugin where the input value will be sent.
        dataId = plugin->getDataId(getConfig(config, "dataId")); //eg: "data_id"

        /// The values that will interract with this component in order to watch them for refreshing the graph. */
        if (config.contains("values") && config["values"].is_array()) { //eg: ["LEVEL", "VOLUME"]
            for (auto& value : config["values"]) {
                watch(plugin->getValue(value.get<std::string>()));
            }
        }

        /// If true, the data is an array. Default is false.
        isArray = config.value("isArrayData", isArray); //eg: false

        /// If true, the data is inverted. Default is false.
        inverted = config.value("inverted", inverted);

        /*md md_config_end */
    }

    std::vector<Point> getPoints() override
    {
        std::vector<Point> points = {};
        if (dataId != -1) {
            float halfHeight = waveformHeight * 0.49;
            points.push_back({ 0, (int)(halfHeight) });

            if (isArray) {
                float* value = (float*)plugin->data(dataId, &size.w);
                for (int i = 0; i < size.w; i++) {
                    points.push_back({ i, (int)(CLAMP(*value, 0.0f, 1.0f) * halfHeight + halfHeight) });
                    value++;
                }
                // printf("\n\n");
            } else {
                for (int i = 0; i < size.w; i++) {
                    float index = i / (float)(size.w - 1);
                    float* value = (float*)plugin->data(dataId, &index);
                    if (value != NULL) {
                        points.push_back({ i, (int)(CLAMP(*value, 0.0f, 1.0f) * halfHeight + halfHeight) });
                    }
                }
            }

            points.push_back({ size.w, (int)(halfHeight) });

            if (inverted) {
                for (auto& point : points) {
                    point.y = waveformHeight - point.y;
                }
            }
        }
        return points;
    }
};
