#ifndef _UI_PIXEL_COMPONENT_GRAPH_ENCODER_H_
#define _UI_PIXEL_COMPONENT_GRAPH_ENCODER_H_

#include "utils/BaseGraphEncoderComponent.h"
#include <stdexcept>

/*md
## GraphEncoderComponent

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/graphEncoder.png" />

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

public:
    GraphEncoderComponent(ComponentInterface::Props props)
        : BaseGraphEncoderComponent(props)
    {
    }

    std::vector<Title> getTitles() override
    {
        std::vector<Title> titles = {};
        for (auto& encoder : encoders) {
            if (encoder.value) {
                if (encoder.value->props().type == VALUE_STRING || encoder.toString) {
                    titles.push_back({ encoder.value->string() });
                } else if (encoder.value->props().unit != NULL) {
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
            for (int i = 0; i < size.w; i++) {
                float index = i / (float)(size.w - 1);
                float* value = (float*)plugin->data(dataId, &index);
                if (value != NULL) {
                    points.push_back({ i, (int)(*value * halfHeight + halfHeight) });
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

    /*md **Config**: */
    /*md - `PLUGIN: plugin_name` set plugin target */
    /*md - `OUTLINE: true/false` is if the envelop should be outlined (default: true). */
    /*md - `FILLED: true/false` is if the envelop should be filled (default: true). */
    /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
    /*md - `FILL_COLOR: color` is the color of the envelop. */
    /*md - `OUTLINE_COLOR: color` is the color of the envelop outline. */
    /*md - `TEXT_COLOR1: color` is the color of the value. */
    /*md - `TEXT_COLOR2: color` is the color of the unit. */
    /*md - `INACTIVE_COLOR_RATIO: 0.0 - 1.0` is the ratio of darkness for the inactive color (default: 0.5). */
    bool config(char* key, char* value)
    {
        /*md - `DATA_ID: data_id` is the data id to get the shape/graph to draw.*/
        if (strcmp(key, "DATA_ID") == 0) {
            if (plugin == NULL) {
                throw std::runtime_error("GraphEncoderComponent cannot set DATA_ID: plugin is not set");
            }
            dataId = plugin->getDataId(value);
            return true;
        }

        /*md - `ENCODER: encoder_id value [string]` is the id of the encoder to update given value, e.g. `ENCODER: 0 LEVEL`. Set `string` to force using string rendering. */
        if (strcmp(key, "ENCODER") == 0) {
            uint8_t encoderId = atoi(strtok(value, " "));
            char* encoderValue = strtok(NULL, " ");
            char* toString = strtok(NULL, " ");
            encoders.push_back({ encoderId,
                watch(plugin->getValue(encoderValue)),
                toString != NULL && strcmp(toString, "string") == 0 });
            return true;
        }

        return BaseGraphEncoderComponent::config(key, value);
    }
};

#endif
