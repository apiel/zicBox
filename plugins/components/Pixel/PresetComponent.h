#pragma once

#include "plugins/audio/utils/fileBrowser.h"
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
    Color scrollColor;

    int fontSize = 12;
    void* font = NULL;

    int encoderId = 1;

    AudioPlugin* audioPlugin = nullptr;
    std::string folder = "data/presets";

    FileBrowser fileBrowser;

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
        , scrollColor(lighten(styles.colors.background, 1.0))
    {
        nlohmann::json& config = props.config;
        audioPlugin = getPluginPtr(config, "audioPlugin", track); //eg: "audio_plugin_name"

        folder = config.value("folder", folder);

        bgColor = draw.getColor(config["bgColor"], bgColor);
        textColor = draw.getColor(config["textColor"], textColor);
        foregroundColor = draw.getColor(config["foregroundColor"], foregroundColor);
        scrollColor = draw.getColor(config["scrollColor"], scrollColor); //eg: "#000000"

        if (config.contains("font")) {
            font = draw.getFont(config["font"].get<std::string>().c_str());
            fontSize = draw.getDefaultFontSize(font);
        }
        fontSize = config.value("fontSize", fontSize);

        encoderId = config.value("encoderId", encoderId);

        fileBrowser.openFolder(folder);
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { bgColor });

        int sep = 4;
        Point topPos = { relativePosition.x, relativePosition.y };
        Size topSize = { size.w, size.h };

        draw.filledRect(topPos, topSize, { foregroundColor });

        if (fileBrowser.count > 1) {
            int scrollW = size.w * ((float)(fileBrowser.position - 1) / (float)(fileBrowser.count - 1));
            draw.filledRect(relativePosition, { scrollW, 2 }, { scrollColor });
        }

        int textY = topPos.y + (size.h - fontSize) * 0.5;
        std::string filename = fileBrowser.getFileWithoutExtension(fileBrowser.position);
        draw.text({ topPos.x + 4, textY }, filename, fontSize, { textEditColor, .font = font });
    }

    void onEncoder(int id, int8_t direction) override
    {
        if (id == encoderId) {
            int position = fileBrowser.position + direction;
            if (position < 1)
                position = 1;
            if (position > fileBrowser.count)
                position = fileBrowser.count;

            fileBrowser.position = position;
            renderNext();
        }
    }
};
