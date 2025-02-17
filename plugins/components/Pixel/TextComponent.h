#pragma once

#include "log.h"
#include "plugins/components/base/Icon.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

/*md
## Text

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/text.png" />

Text component is used to display text.
*/

class TextComponent : public Component {
    Icon icon;

    Color bgColor;
    Color color;

    std::string text;

    bool centered = false;
    int fontSize = 8;
    int fontHeight = 0;
    void* font = NULL;

public:
    TextComponent(ComponentInterface::Props props)
        : Component(props)
        , icon(props.view->draw)
        , bgColor(styles.colors.background)
        , color(styles.colors.text)
    {
        /*md md_config:Text */
        nlohmann::json config = props.config;
        if (!config.contains("text")) {
            logWarn("Text component is missing text parameter.");
        }
        /*md   // The text to display. */
        /*md   text="Hello World" */
        text = config["text"].get<std::string>();
        /*md   // If true, the text will be centered. Default is false. */
        /*md   centered */
        centered = config.value("centered", centered);
        /*md   // The font size of the text. Default is 8. */
        /*md   fontSize=8 */
        fontSize = config.value("fontSize", 8);
        /*md   // The font of the text. Default is null. */
        /*md   font="Sinclair_S" */
        if (config.contains("font")) {
            font = draw.getFont(config["font"].get<std::string>().c_str());
        }
        /*md   // The font height of the text. Default is 0. */
        /*md   fontHeight=0 */
        fontHeight = config.value("fontHeight", fontHeight);
        /*md   // The background color of the text. */
        /*md   bgColor="#000000" */
        bgColor = draw.getColor(config["bgColor"], bgColor);
        
        /*md   // The color of the text */
        /*md   color="#ffffff" */
        color = draw.getColor(config["color"], color);

        /*md md_config_end */
    }

    void render() override
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { bgColor });
            if (!text.empty()) {
                if (centered) {
                    Point textPos = { relativePosition.x + (int)(size.w * 0.5), relativePosition.y + (int)(size.h * 0.5) - 4 };
                    if (!icon.render(text, textPos, 8, { color }, Icon::CENTER)) {

                        draw.textCentered(textPos, text, fontSize, { color, .font = font, .maxWidth = size.w, .fontHeight = fontHeight });
                    }
                } else {
                    if (!icon.render(text, { relativePosition.x, relativePosition.y }, fontSize, { color }, Icon::LEFT)) {
                        draw.text({ relativePosition.x, relativePosition.y }, text, fontSize, { color, .font = font, .maxWidth = size.w, .fontHeight = fontHeight });
                    }
                }
            }
        }
    }

    bool isActive = true;
    void onGroupChanged(int8_t index) override
    {
        bool shouldActivate = false;
        if (group == index || group == -1) {
            shouldActivate = true;
        }
        if (shouldActivate != isActive) {
            isActive = shouldActivate;
            renderNext();
        }
    }
};
