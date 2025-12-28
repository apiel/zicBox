#pragma once

#include "plugins/components/component.h"

#include <string>

/*md
## Clip
*/

class ClipComponent : public Component {
protected:
    Color bgColor;
    Color textColor;

    int fontSize = 12;
    void* font = NULL;

    int smallFontSize = 8;
    void* smallFont = NULL;

    AudioPlugin* pluginSerialize = NULL;
    ValueInterface* valClip = NULL;

public:
    ClipComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
    {
        /*md md_config:Rect */
        nlohmann::json& config = props.config;

        /// The background color.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        /// The color of the text
        textColor = draw.getColor(config["color"], textColor); //eg: "#ffffff"

        /// The font of the text. Default is null.
        if (config.contains("font")) {
            font = draw.getFont(config["font"].get<std::string>().c_str()); //eg: "Sinclair_S"
            fontSize = draw.getDefaultFontSize(font);
        }
        /// The font size of the text.
        fontSize = config.value("fontSize", fontSize); //eg: 8

        /// The font of the text. Default is null.
        if (config.contains("smallFont")) {
            smallFont = draw.getFont(config["smallFont"].get<std::string>().c_str()); //eg: "Sinclair_S"
            smallFontSize = draw.getDefaultFontSize(smallFont);
        }
        /// The font size of the text.
        smallFontSize = config.value("smallFontSize", smallFontSize); //eg: 8

        /// The audio plugin to get serialized data.
        pluginSerialize = &getPlugin(config.value("serializerPlugin", "SerializeTrack"), track); //eg: "audio_plugin_name"
        if (!pluginSerialize) {
            logWarn("Clips component is missing serializerPlugin.");
            return;
        }

        valClip = watch(pluginSerialize->getValue("CLIP"));

        /*md md_config_end */
    }
    void render()
    {
        draw.filledRect(relativePosition, size, { bgColor });

        // int x = draw.text({ relativePosition.x + 2, relativePosition.y }, "Clip:", fontSize, { textColor, .font = font });
        // draw.text({ x + 2, relativePosition.y }, valClip ? std::to_string((int)valClip->get()) : "NULL", fontSize, { textColor, .font = font });
        int x = draw.text({ relativePosition.x + 2, relativePosition.y }, "Clip", smallFontSize, { textColor, .font = smallFont });

        // if play next do 
        // 0 -> 1

        // here should be selection
        draw.textCentered({ relativePosition.x + size.w / 2, relativePosition.y + smallFontSize }, valClip ? std::to_string((int)valClip->get()) : "NULL", fontSize * 2, { textColor, .font = font });
        draw.textCentered({ relativePosition.x + size.w / 2, relativePosition.y + size.h - fontSize - fontSize / 2  }, "clip name", fontSize, { textColor, .font = font, .maxWidth = size.w - 4 });
    }
};
