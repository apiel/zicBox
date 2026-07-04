/** Description:
This component, named `AdsrComponent`, is a specialized visual display element used primarily in audio interfaces, representing the ADSR (Attack, Decay, Sustain, Release) envelope.

**Purpose:** It graphically shows the time-based evolution of a sound, specifically how quickly the sound reaches full volume (Attack), how it settles down (Decay), its sustained level (Sustain), and how long it fades out (Release). It gives the user immediate visual feedback on the sound's contour.

**Mechanism:** The component inherits base drawing capabilities, allowing it to render a flexible curve that illustrates the envelope shape. It internally tracks the four ADSR parameters and uses these values to calculate a series of points, which are then connected to form the characteristic ADSR graph. Subtle mathematical calculations ensure that adjustments feel smooth and musically intuitive.

**Interaction:** The design is highly focused on input control. The component links each of the four envelope stages to physical input devices, such as encoders or knobs. When a user turns a control, the component recognizes the change and instantly updates the corresponding ADSR value in the audio plugin, simultaneously redrawing the graph to show the new shape.

**Setup:** During initialization, the component reads configuration data to determine its visual appearance (like fill color and outline). Crucially, it is told which specific controls (encoders) and audio plugin parameters (values) it must monitor and control, acting as the visual bridge between the user's input and the sound generation engine.

sha: 96e0996a7aba57ee4d5dc0a6265fc8a7ea351cc4c8f1dc826a5fb8bc8882e525 
*/
#pragma once

#include "utils/BaseGraphComponent.h"

/*md
## AdsrComponent

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
        /*md md_config:Adsr */
        nlohmann::json& config = props.config;

        /// If true, the rectangle will be outlined. Default is true.
        outline = config.value("outline", outline); //eg: false

        /// Set the color of the graph.
        fillColor = draw.getColor(config["fillColor"], fillColor); //eg: "#ffffff"

        /// If true, the rectangle will be filled. Default is true.
        filled = config.value("filled", filled); //eg: false

        /// Set the color of the graph outline.
        //md   outlineColor="#000000"

        /// Set the color of the background.
        //md   bgColor="#000000"

        /// If true, the title will be rendered on top of the graph. Default is true.
        //md   renderValuesOnTop={false}

        /// The audio plugin to get control on.
        //md   plugin="audio_plugin_name"

        /// The encoders that will interract with the ADSR envelop.
        //md   encoders={[0, 1, 2, 3]}
        if (config.contains("encoders") && config["encoders"].is_array() && config["encoders"].size() == 4) {
            encoders[0].id = config["encoders"][0].get<int>();
            encoders[1].id = config["encoders"][1].get<int>();
            encoders[2].id = config["encoders"][2].get<int>();
            encoders[3].id = config["encoders"][3].get<int>();
        }

        /// The values that will interract with the ADSR envelop.
        //md   values={["ATTACK", "DECAY", "SUSTAIN", "RELEASE"]}
        if (config.contains("values") && config["values"].is_array() && config["values"].size() == 4) {
            encoders[0].value = watch(plugin->getValue(config["values"][0].get<std::string>()));
            encoders[1].value = watch(plugin->getValue(config["values"][1].get<std::string>()));
            encoders[2].value = watch(plugin->getValue(config["values"][2].get<std::string>()));
            encoders[3].value = watch(plugin->getValue(config["values"][3].get<std::string>()));
        }

        /*md md_config_end */
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

    void onEncoder(int8_t id, int8_t direction) override
    {
        for (auto& encoder : encoders) {
            if (encoder.id == id) {
                encoder.value->increment(direction);
                renderNext();
                break;
            }
        }
    }
};
