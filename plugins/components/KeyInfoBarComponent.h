#ifndef _UI_COMPONENT_KEY_INFO_BAR_H_
#define _UI_COMPONENT_KEY_INFO_BAR_H_

#include "./base/KeypadLayout.h"
#include "component.h"
#include <cmath>
#include <string>
#include <vector>

/*md
## KeyInfoBar

KeyInfoBar components provide some information about the current kaypad layout.

*/
class KeyInfoBarComponent : public Component {
protected:
    Size itemSize = { 0, 0 };
    int itemColumnCount = 5;
    int itemRowCount = 2;
    int fontSize = 9;
    int textTopMargin = 0;
    int textLeftMargin = 0;

    KeypadLayout keypadLayout;
    int8_t activeItem = -1;

    struct Item {
        std::string text;
        std::string title;
        bool needUpdate = true;
        std::vector<std::string> children; // but then needUpdate might not work anymore...
    };

    std::vector<Item> items;

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

        draw.filledRect(pos, itemSize, background);
        draw.textCentered({ pos.x + textLeftMargin, pos.y }, items[index].text, fontColor, fontSize);
    }

    void handleButton(int8_t id, int8_t state)
    {
        items[activeItem].needUpdate = true;
        if (state == 0) {
            if ( items[activeItem].children.size() ) {
                for (int i = 0; i < items[activeItem].children.size(); i++) {
                    items[i].text = items[i].title;
                    items[i].needUpdate = true;
                }
            }
            activeItem = -1;
        } else {
            activeItem = id;
            if ( items[activeItem].children.size() ) {
                for (int i = 0; i < items[activeItem].children.size(); i++) {
                    items[i].text = items[activeItem].children[i];
                    items[i].needUpdate = true;
                }
            }
        }
        renderNext();
    }

    struct Colors {
        Color background;
        Color font;
        Color activeBackground;
        Color activeFont;
    } colors;

public:
    /*md **Keyboard actions**: */
    void addKeyMap(KeypadInterface* controller, uint16_t controllerId, uint8_t key, int param, std::string action, uint8_t color)
    {
        /*md - `item` to select item. */
        if (action == "item") {
            keypadLayout.mapping.push_back(
                { controller, controllerId, key, param,
                    [&](int8_t state, KeypadLayout::KeyMap& keymap) { handleButton(keymap.param, state); },
                    color, [&](KeypadLayout::KeyMap& keymap) { return keymap.color == 255 ? 10 : keymap.color; } });
        }
    }

    KeyInfoBarComponent(ComponentInterface::Props props)
        : Component(props)
        , keypadLayout(getController, [&](KeypadInterface* controller, uint16_t controllerId, int8_t key, int param, std::string action, uint8_t color) { addKeyMap(controller, controllerId, key, param, action, color); })
    {
        colors.background = props.draw.lighten(props.draw.styles.colors.background, 0.2);
        colors.font = props.draw.styles.colors.grey;
        colors.activeBackground = props.draw.lighten(colors.background, 0.4);
        colors.activeFont = props.draw.lighten(colors.font, 0.4);
        // resize(); // No need to call when empty
    }

    void resize()
    {
        if (items.size()) {
            itemRowCount = std::ceil(items.size() / (float)itemColumnCount);
            itemSize = { (size.w / itemColumnCount) - 1, (size.h / itemRowCount) - 1 };
            textTopMargin = (itemSize.h - fontSize) / 2;
            textLeftMargin = itemSize.w / 2; // center
        }
    }

    void render()
    {
        for (int row = 0; row < itemRowCount; row++) {
            for (int col = 0; col < itemColumnCount; col++) {
                int itemIndex = row * itemColumnCount + col;
                if (items[itemIndex].needUpdate) {
                    Point pos = { position.x + col * (itemSize.w + 1), position.y + row * (itemSize.h + 1) };
                    renderItem(pos, itemIndex);
                }
            }
        }
    }

    bool config(char* key, char* value)
    {
        /*md - `BACKGROUND: #333333` set the background color and activate background drawing. */
        if (strcmp(key, "BACKGROUND") == 0) {
            colors.background = draw.getColor(value);
            colors.activeBackground = draw.lighten(colors.background, 0.4);
            return true;
        }

        /*md - `FONT_COLOR: #ffffff` set the font color. */
        if (strcmp(key, "FONT_COLOR") == 0) {
            colors.font = draw.getColor(value);
            colors.activeFont = draw.lighten(colors.font, 0.4);
            return true;
        }

        /*md - `FONT_SIZE: 10` set the font size. */
        if (strcmp(key, "FONT_SIZE") == 0) {
            fontSize = atoi(value);
            textTopMargin = (itemSize.h - fontSize) / 2;
            return true;
        }

        /*md - `COLUMNS: 5` set the number of columns. */
        if (strcmp(key, "COLUMNS") == 0) {
            itemColumnCount = atoi(value);
            resize();
            return true;
        }

        /*md - `ITEM: name` add new item to list. */
        if (strcmp(key, "ITEM") == 0) {
            Item item;
            item.text = value;
            item.title = value;
            items.push_back(item);
            resize();
            return true;
        }

        /*md - `CHILD: name` add child to last item. */
        if (strcmp(key, "CHILD") == 0) {
            if (items.size()) {
                items[items.size() - 1].children.push_back(value);
            }
            return true;
        }

        if (keypadLayout.config(key, value)) {
            return true;
        }

        return false;
    }

    void onKey(uint16_t id, int key, int8_t state)
    {
        // printf("onKey %d %d %d\n", id, key, state);
        keypadLayout.onKey(id, key, state);
    }

    void initView(uint16_t counter)
    {
        keypadLayout.renderKeypad();
    }
};

#endif
