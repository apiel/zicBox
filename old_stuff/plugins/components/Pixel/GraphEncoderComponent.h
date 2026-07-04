/** Description:
This document explains the functionality of the `GraphEncoderComponent`, a building block used in graphical interfaces, particularly those dealing with audio processing or signal visualization.

### I. Purpose and Role

The `GraphEncoderComponent` is a specialized display and control element designed to visualize numerical data as a dynamic graph or waveform. Its primary function is to show a representation of data points and provide a way for the user to potentially interact with and control those points. It extends a basic graph framework, inheriting standard drawing capabilities while adding specific data handling logic.

### II. Initialization and Configuration

When the component is created, it reads a detailed set of instructions (configuration) that dictates both its appearance and its data source:

1.  **Visual Settings:** It configures aspects like the graph’s color, whether the shape is filled or just outlined, the color of the outline, and the background color.
2.  **Data Source Link:** Crucially, it must be linked to an external "audio plugin" and a specific data channel or identifier within that plugin. This link determines where the component retrieves its numerical information.
3.  **Data Watching:** It is set up to "watch" specific values from the plugin, ensuring the graph refreshes immediately whenever that underlying data changes.
4.  **Behavior Flags:** It reads flags to determine if the input data is treated as a continuous block of numbers (an array) or sequential single values. It also checks if the visualization needs to be vertically reversed (inverted).

### III. Core Functionality (Data Visualization)

The core mechanism involves constantly calculating the points required to draw the graph:

1.  **Data Retrieval:** The component queries the connected audio plugin for the current values associated with the assigned data channel.
2.  **Point Calculation:** It iterates across its drawing area (its width) and maps the retrieved data values to specific vertical coordinates. It ensures these coordinates are properly scaled to fit within the component’s boundaries.
3.  **Handling Different Data Types:**
    *   If the component expects **array data**, it fetches a whole block of values from the plugin and uses those sequential numbers to draw the shape point-by-point.
    *   If it expects **single values**, it queries the plugin repeatedly, asking for the value corresponding to each horizontal position, creating a line based on sequential inputs.
4.  **Display Adjustment:** If the inversion setting is active, it flips the calculated points vertically before drawing, ensuring the graph is displayed in the desired orientation.

In essence, this component serves as a dynamic visual bridge, converting raw plugin data into a real-time, configurable graph for the user interface.

sha: 1e500f1d3cf52b12c0d611c662de8d4b7b4cb616d2930900daf4700a397010d5 
*/
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
