#pragma once

#include "plugins/components/component.h"

#include <string>

/*md
## FmAlgo

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo1.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo2.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo3.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo4.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo5.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo6.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo7.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo8.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo9.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo10.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo11.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo12.png" />

Show FM algorithm and change them. The filled square are the carrier where audio is outputted and the not filled square are the operators modulating the frequency.
*/
class FmAlgoComponent : public Component {
protected:
    Color background;
    Color border;
    Color text;

    Size opSize = { 16, 16 };

    uint8_t fontSize = 8;
    int8_t encoderId = -1;
    ValueInterface* value = NULL;
    AudioPlugin* plugin;
    int8_t dataId = 0;

    int textMarginTop = 4;
    int textMarginLeft = 1;

public:
    FmAlgoComponent(ComponentInterface::Props props)
        : Component(props)
        , background(styles.colors.background)
        , border({ 0x80, 0x80, 0x80 })
        , text(styles.colors.primary)
    {
        if (size.w > size.h) {
            position.x += (size.w - size.h) * 0.5f;
            size.w = size.h;
        } else if (size.h > size.w) {
            position.y += (size.h - size.w) * 0.5f;
            size.h = size.w;
        }

        /*md md_config:FmAlgo */
        nlohmann::json& config = props.config;

        /// The background color of the text.
        background = draw.getColor(config["bgColor"], background); //eg: "#000000"

        /// The color of the text.
        text = draw.getColor(config["textColor"], text); //eg: "#ffffff"

        /// The color of the border.
        border = draw.getColor(config["borderColor"], border); //eg: "#ffffff"

        /// The encoder id that will interract with this component.
        encoderId = config["encoderId"].get<int8_t>(); //eg: 0

        /// The audio plugin to get control on.
        plugin = getPluginPtr(config, "audioPlugin", track); //eg: "audio_plugin_name"

        /// The audio plugin key parameter to get control on.
        std::string param = getConfig(config, "param"); //eg: "parameter_name"
        value = watch(plugin->getValue(param));

        /// The data id that will return the current algorithm layout.
        dataId = plugin->getDataId(config.value("dataId", "ALGO")); //eg: "ALGO"

        /*md md_config_end */
    }

    void render()
    {
        // printf("updated position %d value=%f\n", dataId, value->get());
        draw.filledRect(relativePosition, size, { background });

        bool(*algo)[3] = (bool(*)[3])plugin->data(dataId);

        // draw modulation link
        if (algo[0][0]) // 1 to 2
        {
            Point start = { relativePosition.x + opSize.w, (int)(relativePosition.y + opSize.h * 0.5) };
            Point dest = { relativePosition.x + size.w - opSize.w, (int)(relativePosition.y + opSize.h * 0.5) };
            draw.line(start, dest, { text });

            draw.line({ dest.x - 4, dest.y - 4 }, dest, { text });
            draw.line({ dest.x - 4, dest.y + 4 }, dest, { text });
        }
        if (algo[1][1]) // 2 to 3
        {
            Point start = { relativePosition.x + size.w - opSize.w, relativePosition.y + opSize.h };
            Point dest = { relativePosition.x + opSize.w, relativePosition.y + size.h - opSize.h };
            draw.line(start, dest, { text });

            draw.line({ dest.x, dest.y - 5 }, dest, { text });
            draw.line({ dest.x + 5, dest.y }, dest, { text });
        }
        if (algo[2][2]) // 3 to 4
        {
            Point start = { relativePosition.x + opSize.w, (int)(relativePosition.y + size.h - opSize.h * 0.5) };
            Point dest = { relativePosition.x + size.w - opSize.w, (int)(relativePosition.y + size.h - opSize.h * 0.5) };
            draw.line(start, dest, { text });

            draw.line({ dest.x - 4, dest.y - 4 }, dest, { text });
            draw.line({ dest.x - 4, dest.y + 4 }, dest, { text });
        }
        if (algo[0][1]) // 1 to 3
        {
            Point start = { (int)(relativePosition.x + opSize.w * 0.5), relativePosition.y + opSize.h };
            Point dest = { (int)(relativePosition.x + opSize.w * 0.5), relativePosition.y + size.h - opSize.h };
            draw.line(start, dest, { text });

            draw.line({ dest.x - 4, dest.y - 4 }, dest, { text });
            draw.line({ dest.x + 4, dest.y - 4 }, dest, { text });
        }
        if (algo[0][2]) // 1 to 4
        {
            Point start = { relativePosition.x + opSize.w, relativePosition.y + opSize.h };
            Point dest = { relativePosition.x + size.w - opSize.w, relativePosition.y + size.h - opSize.h };
            draw.line(start, dest, { text });

            draw.line({ dest.x, dest.y - 5 }, dest, { text });
            draw.line({ dest.x - 5, dest.y }, dest, { text });
        }
        if (algo[1][2]) // 2 to 4
        {
            Point start = { (int)(relativePosition.x + size.w - opSize.w * 0.5), relativePosition.y + opSize.h };
            Point end = { (int)(relativePosition.x + size.w - opSize.w * 0.5), relativePosition.y + size.h - opSize.h };
            draw.line(start, end, { text });

            draw.line({ end.x - 4, end.y - 4 }, end, { text });
            draw.line({ end.x + 4, end.y - 4 }, end, { text });
        }

        // draw operators and carriers
        // 4 is always carrier
        draw.filledRect({ relativePosition.x + size.w - opSize.w, relativePosition.y + size.h - opSize.h }, opSize, { border }); // 4
        draw.textCentered({ (int)(relativePosition.x + size.w - opSize.w + opSize.w * 0.5 + textMarginLeft), relativePosition.y + size.h - opSize.h + textMarginTop }, "4", fontSize, { background });

        if (!algo[0][0] && !algo[0][1] && !algo[0][2]) { // 1
            draw.filledRect(position, opSize, { border }); // 1
            draw.textCentered({ (int)(relativePosition.x + opSize.w * 0.5 + textMarginLeft), relativePosition.y + textMarginTop }, "1", fontSize, { background });
        } else {
            draw.rect(position, opSize, { border }); // 1
            draw.textCentered({ (int)(relativePosition.x + opSize.w * 0.5 + textMarginLeft), relativePosition.y + textMarginTop }, "1", fontSize, { text });
        }

        if (!algo[1][0] && !algo[1][1] && !algo[1][2]) { // 2
            draw.filledRect({ relativePosition.x + size.w - opSize.w, relativePosition.y }, opSize, { border }); // 2
            draw.textCentered({ (int)(relativePosition.x + size.w - opSize.w + opSize.w * 0.5 + textMarginLeft), relativePosition.y + textMarginTop }, "2", fontSize, { background });
        } else {
            draw.rect({ relativePosition.x + size.w - opSize.w, relativePosition.y }, opSize, { border }); // 2
            draw.textCentered({ (int)(relativePosition.x + size.w - opSize.w + opSize.w * 0.5 + textMarginLeft), relativePosition.y + textMarginTop }, "2", fontSize, { text });
        }

        if (!algo[2][0] && !algo[2][1] && !algo[2][2]) { // 3
            draw.filledRect({ relativePosition.x, relativePosition.y + size.h - opSize.h }, opSize, { border }); // 3
            draw.textCentered({ (int)(relativePosition.x + opSize.w * 0.5 + textMarginLeft), relativePosition.y + size.h - opSize.h + textMarginTop }, "3", fontSize, { background });
        } else {
            draw.rect({ relativePosition.x, relativePosition.y + size.h - opSize.h }, opSize, { border }); // 3
            draw.textCentered({ (int)(relativePosition.x + opSize.w * 0.5 + textMarginLeft), relativePosition.y + size.h - opSize.h + textMarginTop }, "3", fontSize, { text });
        }
    }

    void onEncoder(int8_t id, int8_t direction)
    {
        if (value && id == encoderId) {
            value->increment(direction);
        }
    }
};
