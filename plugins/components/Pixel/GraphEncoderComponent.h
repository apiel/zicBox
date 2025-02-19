#ifndef _UI_PIXEL_COMPONENT_GRAPH_ENCODER_H_
#define _UI_PIXEL_COMPONENT_GRAPH_ENCODER_H_

#include "utils/BaseGraphEncoderComponent.h"
#include <stdexcept>

/*md
## GraphEncoderComponent

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/graphEncoder.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/graphEncoder2.png" />

Show a representation of data points and provide a way to control them.

*/

class GraphEncoderComponent : public BaseGraphEncoderComponent {
    struct EncoderParam {
        int id = -1;
        ValueInterface* value = NULL;
        bool toString = false;
    };
    std::vector<EncoderParam> encoders;

    uint8_t dataId = -1;

    bool isArray = false;

public:
    GraphEncoderComponent(ComponentInterface::Props props)
        : BaseGraphEncoderComponent(props)
    {
        /*md md_config:GraphEncoder */
        nlohmann::json config = props.config;

        /// If true, the rectangle will be outlined. Default is true.
        //md   outline={false}

        /// Set the color of the graph.
        //md   fillColor="#000000"

        /// If true, the rectangle will be filled. Default is true.
        //md   filled={false}

        /// Set the color of the graph outline.
        //md   outlineColor="#000000"

        /// Set the color of the text.
        //md   textColor1="#000000"

        /// Set the color of the unit.
        //md   textColor2="#000000"

        /// Set the color of the background.
        //md   bgColor="#000000"

        /// If true, the title will be rendered on top of the graph. Default is true.
        //md   renderValuesOnTop={false}

        /// The audio plugin to get control on.
        //md   plugin="audio_plugin_name"

        if (plugin == NULL) {
            logWarn("GraphEncoderComponent cannot init: plugin is not set");
            return;
        }

        /// The data id of the audio plugin where the input value will be sent.
        dataId = plugin->getDataId(getConfig(config, "dataId")); //eg: "data_id"

        /// The encoders that will interract with this component. Set `string` to force using string rendering. */
        if (config.contains("encoders") && config["encoders"].is_array()) { //eg: [{encoderId: 0, value: "LEVEL", string: false}]
            for (auto& encoder : config["encoders"]) {
                encoders.push_back({ encoder["encoderId"].get<int>(),
                    watch(plugin->getValue(encoder["value"].get<std::string>())),
                    encoder.contains("string") && encoder["string"].get<bool>() });
            }
        }

        /// If true, the data is an array. Default is false.
        isArray = config.value("isArrayData", isArray); //eg: false

        /*md md_config_end */
    }

    std::vector<Title> getTitles() override
    {
        std::vector<Title> titles = {};
        for (auto& encoder : encoders) {
            if (encoder.value) {
                if (encoder.value->props().type == VALUE_STRING || encoder.toString) {
                    titles.push_back({ encoder.value->string() });
                } else if (encoder.value->props().unit.length() > 0) {
                    titles.push_back({ std::to_string(encoder.value->get()), encoder.value->props().unit });
                } else {
                    titles.push_back({ std::to_string(encoder.value->get()) });
                }
            }
        }
        return titles;
    }

    std::vector<Point> getPoints() override
    {
        std::vector<Point> points = {};
        if (dataId != -1) {
            float halfHeight = waveformHeight * 0.5;
            points.push_back({ 0, (int)(halfHeight) });

            if (isArray) {
                float* value = (float*)plugin->data(dataId, &size.w);
                for (int i = 0; i < size.w; i++) {
                    points.push_back({ i, (int)(*value * halfHeight + halfHeight) });
                    // printf("%f,", *value);
                    value++;
                }
                // printf("\n\n");
            } else {
                for (int i = 0; i < size.w; i++) {
                    float index = i / (float)(size.w - 1);
                    float* value = (float*)plugin->data(dataId, &index);
                    if (value != NULL) {
                        // printf("[%d:%f] value %f\n", i, index, *value);
                        points.push_back({ i, (int)(*value * halfHeight + halfHeight) });
                    }
                }
            }

            points.push_back({ size.w, (int)(halfHeight) });
        }
        return points;
    }

    void onEncoder(int id, int8_t direction) override
    {
        if (isActive) {
            for (auto& encoder : encoders) {
                if (encoder.id == id) {
                    encoder.value->increment(direction);
                    renderNext();
                    break;
                }
            }
        }
    }
};

#endif
