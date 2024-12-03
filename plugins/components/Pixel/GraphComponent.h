#ifndef _UI_PIXEL_COMPONENT_GRAPH_H_
#define _UI_PIXEL_COMPONENT_GRAPH_H_

#include "../../../helpers/range.h"
#include "../component.h"
#include "../utils/color.h"
#include "utils/GroupColorComponent.h"

/*md
## GraphComponent

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/graph.png" />

Show a representation of data points and provide a way to control them.

*/

class GraphComponent : public GroupColorComponent {
    AudioPlugin* plugin = NULL;

    bool filled = true;
    bool outline = true;

    Color bgColor;

    ToggleColor fillColor;
    ToggleColor outlineColor;
    ToggleColor textColor;

    struct EncoderParam {
        int id = -1;
        ValueInterface* value = NULL;
        bool toString = false;
    };
    std::vector<EncoderParam> encoders;

    uint8_t dataId = -1;

    int waveformHeight = 30;
    int waveformY = 0;

    void renderGraph()
    {
        if (dataId != -1) {
            std::vector<Point> relativePoints;
            float halfHeight = waveformHeight * 0.5;
            relativePoints.push_back({ relativePosition.x, (int)(waveformY + halfHeight) });
            for (int i = 0; i < size.w; i++) {
                float index = i / (float)(size.w - 1);
                float* value = (float*)plugin->data(dataId, &index);
                Point point = { i, (int)(*value * halfHeight + halfHeight) };
                relativePoints.push_back({ point.x + relativePosition.x, point.y + waveformY });
            }
            relativePoints.push_back({ relativePosition.x + size.w, (int)(waveformY + halfHeight) });
            if (filled) {
                draw.filledPolygon(relativePoints, { fillColor.color });
            }
            if (outline) {
                draw.lines(relativePoints, { outlineColor.color });
            }
        }
    }

    void renderTitles()
    {
        int cellWidth = size.w / encoders.size();
        int x = relativePosition.x + cellWidth * 0.5;
        for (int i = 0; i < encoders.size(); i++) {
            ValueInterface* value = encoders[i].value;
            if (value) {
                if (value->props().type == VALUE_STRING || encoders[i].toString) {
                    draw.textCentered({ x + cellWidth * i, relativePosition.y }, value->string(), 8, { textColor.color });
                } else if (value->props().unit != NULL) {
                    // could be great make unit in a different color
                    draw.textCentered({ x + cellWidth * i, relativePosition.y }, std::to_string(value->get()) + value->props().unit, 8, { textColor.color });
                } else {
                    draw.textCentered({ x + cellWidth * i, relativePosition.y }, std::to_string(value->get()), 8, { textColor.color });
                }
            }
        }
    }

public:
    GraphComponent(ComponentInterface::Props props)
        : GroupColorComponent(props, { { "FILL_COLOR", &fillColor }, { "OUTLINE_COLOR", &outlineColor }, { "TEXT_COLOR", &textColor } })
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text, inactiveColorRatio)
        , fillColor(styles.colors.primary, inactiveColorRatio)
        , outlineColor(lighten(styles.colors.primary, 0.5), inactiveColorRatio)
    {
        updateColors();
        waveformHeight = size.h - 9;
    }

    void render() override
    {
        if (updatePosition()) {
            waveformY = relativePosition.y + 8;
            draw.filledRect(relativePosition, size, { bgColor });
            renderGraph();
            renderTitles();
        }
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
    bool config(char* key, char* value)
    {
        /*md - `PLUGIN: plugin_name` set plugin target */
        if (strcmp(key, "PLUGIN") == 0) {
            plugin = &getPlugin(value, track);
            return true;
        }

        /*md - `DATA_ID: data_id` is the data id to get the shape/graph to draw.*/
        if (strcmp(key, "DATA_ID") == 0) {
            dataId = atoi(value);
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

        /*md - `OUTLINE: true/false` is if the envelop should be outlined (default: true). */
        if (strcmp(key, "OUTLINE") == 0) {
            outline = strcmp(value, "true") == 0;
            return true;
        }

        /*md - `FILLED: true/false` is if the envelop should be filled (default: true). */
        if (strcmp(key, "FILLED") == 0) {
            filled = strcmp(value, "true") == 0;
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(value);
            return true;
        }

        /*md - `FILL_COLOR: color` is the color of the envelop. */
        /*md - `OUTLINE_COLOR: color` is the color of the envelop outline. */
        /*md - `TEXT_COLOR: color` is the color of the text. */
        /*md - `INACTIVE_COLOR_RATIO: 0.0 - 1.0` is the ratio of darkness for the inactive color (default: 0.5). */
        return GroupColorComponent::config(key, value);
    }
};

#endif
