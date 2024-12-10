#ifndef _UI_PIXEL_COMPONENT_ADSR_H_
#define _UI_PIXEL_COMPONENT_ADSR_H_

#include "utils/BaseGraphComponent.h"

/*md
## GraphComponent

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/adsr.png" />

Show a representation of the ADSR envdelop.

*/

class AdsrComponent : public BaseGraphComponent {
    struct EncoderParam {
        std::string name;
        int id = -1;
        ValueInterface* value = NULL;
    };
    std::vector<EncoderParam> encoders = { { "A.", 0 }, { "D.", 1 }, { "S.", 2 }, { "R.", 3 } };

public:
    AdsrComponent(ComponentInterface::Props props)
        : BaseGraphComponent(props)
    {
        textColor1.setColor(styles.colors.secondary, inactiveColorRatio);
        textColor2.setColor(styles.colors.text, inactiveColorRatio);
    }

    std::vector<Title> getTitles() override
    {
        std::vector<Title> titles = {};
        for (auto& encoder : encoders) {
            titles.push_back({ encoder.name, std::to_string((int)encoder.value->get()) + encoder.value->props().unit });
        }
        return titles;
    }

    std::vector<Point> getPoints() override
    {
        std::vector<Point> points = {};
        points.push_back({ 0, waveformHeight });
        // int a = encoders[0].value->pct() * size.w * 0.30;
        // 1 - Math.pow(1 - 0.1, 2) // to make lower value larger and higher value smaller 0.1 = 0.19 & 0.9 = 0.99
        int a = (1 - pow(1 - encoders[0].value->pct(), 10)) * size.w * 0.32;
        points.push_back({ a, 0 });
        // int d = encoders[1].value->pct() * size.w * 0.30;
        int d = (1 - pow(1 - encoders[1].value->pct(), 10)) * size.w * 0.32;
        points.push_back({ a + d, (int)(waveformHeight - encoders[2].value->pct() * waveformHeight) });
        // int r = encoders[3].value->pct() * size.w * 0.30;
        int r = (1 - pow(1 - encoders[3].value->pct(), 10)) * size.w * 0.32;
        points.push_back({ (int)(size.w - r), (int)(waveformHeight - encoders[2].value->pct() * waveformHeight) });
        points.push_back({ size.w, waveformHeight });
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
        /*md - `ENCODERS: A_encoder_id D_encoder_id S_encoder_id R_encoder_id` is the id of the encoder to update given value. */
        if (strcmp(key, "ENCODERS") == 0) {
            encoders[0].id = atoi(strtok(value, " "));
            encoders[1].id = atoi(strtok(NULL, " "));
            encoders[2].id = atoi(strtok(NULL, " "));
            encoders[3].id = atoi(strtok(NULL, " "));

            return true;
        }

        /*md - `VALUES: A_value D_value S_value R_value` are the values id for the encoders and data point. */
        if (strcmp(key, "VALUES") == 0) {
            encoders[0].value = watch(plugin->getValue(strtok(value, " ")));
            encoders[1].value = watch(plugin->getValue(strtok(NULL, " ")));
            encoders[2].value = watch(plugin->getValue(strtok(NULL, " ")));
            encoders[3].value = watch(plugin->getValue(strtok(NULL, " ")));
            return true;
        }

        return BaseGraphComponent::config(key, value);
    }
};

#endif
