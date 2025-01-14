#ifndef _UI_COMPONENT_RECT_H_
#define _UI_COMPONENT_RECT_H_

#include "../base/KeypadLayout.h"
#include "../component.h"
#include "../utils/color.h"
#include <string>
#include <vector>

/*md
## SDL ButtonBar

Button bar components dynamically give label to some push buttons.

> [!NOTE] WIP WIP work in progress...
*/
class ButtonBarComponent : public Component {
protected:
    Size itemSize = { 0, 0 };
    int itemColumnCount = 4;
    int itemRowCount = 2;
    int fontSize = 9;
    int textTopMargin = 2;
    int textLeftMargin = 0;

    KeypadLayout keypadLayout;
    int8_t activeItem = 1;

    struct Item {
        std::string text;
        std::string viewPattern;
        int8_t page;
    };

    int8_t track = 0;

    std::vector<Item> items = {
        { "Track", "track_$track_$page", 0 }, // track_tracknumber_viewnumber
        { "Sequencer", "sequencer_$track_$page", 0 }, // sequencer_tracknumber_viewnumber
        { "Filter", "filter_$track_$page", 0 },
        { "Modulation", "modulation_$track_$page" },
        { "Waveform", "waveform_$track_$page", 0 },
        { "FX", "fx_$track_$page", 0 },
        { "Master", "master_$page", 0 },
        { "...", "misc_$page", 0 },
    };

    void renderItem(Point pos, int8_t index)
    {
        if (index < 0 || index >= items.size()) {
            return;
        }

        Color background = colors.background;
        Color fontColor = colors.font;
        if (index == activeItem) {
            background = colors.activeBackground;
            fontColor = colors.activeFont;
        }

        draw.filledRect(pos, itemSize, { background });
        draw.textCentered({ pos.x + textLeftMargin, pos.y }, items[index].text, fontSize, { fontColor });
    }

    // int8_t previousActiveItem = 1; // to restore after pressing shift
    void handleButton(int8_t id, int8_t state)
    {
        // printf("handleButton: %d %d\n", id, state);
        activeItem = id;
        renderNext();
    }

    struct Colors {
        Color background;
        Color font;
        Color activeBackground;
        Color activeFont;
    } colors;

public:
    ButtonBarComponent(ComponentInterface::Props props)
        : Component(props)
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            /*md **Keyboard actions**: */
            /*md - `item` to select item. */
            if (action.rfind("item:") == 0) {
                int* paramFn = new int(atoi(action.substr(5).c_str()));
                func = [this, paramFn](KeypadLayout::KeyMap& keymap) { handleButton(*paramFn, keymap.pressedTime != -1); };
            }
            return func;
        })
    {
        colors.background = lighten(styles.colors.background, 0.2);
        colors.font = { 0x80, 0x80, 0x80, 255 };
        colors.activeBackground = lighten(colors.background, 0.4);
        colors.activeFont = lighten(colors.font, 0.4);
        itemSize = { (size.w / itemColumnCount) - 1, (size.h / itemRowCount) - 1 };
        textTopMargin = (itemSize.h - fontSize) / 2;
        textLeftMargin = itemSize.w / 2; // center
    }

    void render()
    {
        for (int row = 0; row < itemRowCount; row++) {
            for (int col = 0; col < itemColumnCount; col++) {
                Point pos = { position.x + col * (itemSize.w + 1), position.y + row * (itemSize.h + 1) };
                renderItem(pos, row * itemColumnCount + col);
            }
        }
    }

    bool config(char* key, char* value)
    {
        /*md - `BACKGROUND: #333333` set the background color and activate background drawing. */
        if (strcmp(key, "BACKGROUND") == 0) {
            colors.background = draw.getColor(value);
            colors.activeBackground = lighten(colors.background, 0.4);
            return true;
        }

        /*md - `FONT_COLOR: #ffffff` set the font color. */
        if (strcmp(key, "FONT_COLOR") == 0) {
            colors.font = draw.getColor(value);
            colors.activeFont = lighten(colors.font, 0.4);
            return true;
        }

        /*md - `FONT_SIZE: 10` set the font size. */
        if (strcmp(key, "FONT_SIZE") == 0) {
            fontSize = atoi(value);
            textTopMargin = (itemSize.h - fontSize) / 2;
            return true;
        }

        // /*//md - `ITEM: view label` add an item. */
        // if (strcmp(key, "ITEM") == 0) {
        //     Item item;
        //     item.view = strtok(value, " ");
        //     item.text = strtok(NULL, "");
        //     items.push_back(item);
        //     return true;
        // }

        if (keypadLayout.config(key, value)) {
            return true;
        }

        return false;
    }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        // printf("onKey %d %d %d\n", id, key, state);
        keypadLayout.onKey(id, key, state, now);
    }

    void initView(uint16_t counter)
    {
        keypadLayout.renderKeypadColor();
    }
};

#endif
