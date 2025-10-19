#pragma once

#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <fstream>
#include <string>

class PresetComponent : public Component {
protected:
    Color bgColor;
    Color textColor;
    Color textEditColor;
    Color foregroundColor;

    int fontSize = 12;
    void* font = NULL;
    int cursorHeight = 2;

    AudioPlugin* audioPlugin = nullptr;
    std::string folder = "data/presets";

public:
    PresetComponent(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;

            return func;
        })
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
        , textEditColor(darken(styles.colors.text, 0.5))
        , foregroundColor(lighten(styles.colors.background, 0.5))
    {
        nlohmann::json& config = props.config;
        audioPlugin = getPluginPtr(config, "audioPlugin", track); //eg: "audio_plugin_name"

        folder = config.value("folder", folder);

        bgColor = draw.getColor(config["bgColor"], bgColor);
        textColor = draw.getColor(config["textColor"], textColor);
        foregroundColor = draw.getColor(config["foregroundColor"], foregroundColor);

        if (config.contains("font")) {
            font = draw.getFont(config["font"].get<std::string>().c_str());
            fontSize = draw.getDefaultFontSize(font);
        }
        fontSize = config.value("fontSize", fontSize);
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { bgColor });

        int sep = 4;
        Point topPos = { relativePosition.x, relativePosition.y };
        Size topSize = { size.w, size.h };

        draw.filledRect(topPos, topSize, { foregroundColor });
        int textY = topPos.y + (size.h - fontSize) * 0.5;
    }
};
