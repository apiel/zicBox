#pragma once

#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"
#include "log.h"

/*md
## Text

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/text.png" />

Text component is used to display text.
*/

class TextComponent : public Component {
    Color bgColor;
    Color color;

    std::string text;

    bool centered = false;
    int fontSize = 8;
    int fontHeight = 0;
    void* font = NULL;

    KeypadLayout keypadLayout;

public:
    TextComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , color(darken(styles.colors.text, 0.5))
        , keypadLayout(this)
    {
        /*md 
**Config**: 
```tsx
<Text
*/
        nlohmann::json config = props.config;
        if (!config.contains("text")) {
            logWarn("Text component is missing text parameter.");
        }
        /*md    // The text to display. */
        /*md    text="Hello World" */
        text = config["text"].get<std::string>();
        /*md    // If true, the text will be centered. Default is false. */
        /*md    centered=true */
        /*md    // or simply */
        /*md    centered */
        centered = config.value("centered", false);
/*md
/>
```
*/
    }

    void render() override
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { bgColor });
            if (!text.empty()) {
                if (centered) {
                    Point textPos = { relativePosition.x + (int)(size.w * 0.5), relativePosition.y + (int)(size.h * 0.5) - 4 };
                    draw.textCentered(textPos, text, fontSize, { color, .font = font, .fontHeight = fontHeight });
                } else {
                    draw.text({ relativePosition.x, relativePosition.y }, text, fontSize, { color, .font = font, .fontHeight = fontHeight });
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

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        if (isActive) {
            keypadLayout.onKey(id, key, state, now);
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        if (keypadLayout.config(key, value)) {
            return true;
        }

        /*md - `TEXT: text` is the text of the component. */
        if (strcmp(key, "TEXT") == 0) {
            text = value;
            return true;
        }

        /*md - `CENTERED: true/false` is if the text should be centered (default: false). */
        if (strcmp(key, "CENTERED") == 0) {
            centered = strcmp(value, "true") == 0;
            return true;
        }

        /*md - `FONT_SIZE: size` is the font size of the component. */
        if (strcmp(key, "FONT_SIZE") == 0) {
            fontSize = atoi(value);
            return true;
        }

        /*md - `FONT: font` is the font of the component. */
        if (strcmp(key, "FONT") == 0) {
            font = draw.getFont(value);
            return true;
        }

        /*md - `FONT_HEIGHT: height` is the font height of the component. */
        if (strcmp(key, "FONT_HEIGHT") == 0) {
            fontHeight = atoi(value);
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(value);
            return true;
        }

        /*md - `COLOR: color` is the color of the component. */
        if (strcmp(key, "COLOR") == 0) {
            color = draw.getColor(value);
            return true;
        }

        return false;
    }
};
