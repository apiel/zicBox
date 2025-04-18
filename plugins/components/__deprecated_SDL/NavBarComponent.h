#ifndef _UI_COMPONENT_NAVBAR_H_
#define _UI_COMPONENT_NAVBAR_H_

#include "../base/KeypadLayout.h"
#include "../component.h"
#include <string>
#include <vector>

/*md
## SDL NavBar

Navigation bar components draw a scrollable menu to be used with buttons/keys.

> [!NOTE] WIP WIP work in progress...
*/
class NavBarComponent : public Component {
protected:
    Size arrowSize = { 10, 10 };
    Size itemSize = { 0, 0 };
    int itemsFix = 0;
    Size subItemSize = { 0, 0 };
    int subItemsFix = 0;
    int fontSize = 10;
    int textTopMargin = 2;
    int itemPos = 0;
    bool showSub = false;

    KeypadLayout keypadLayout;

    struct Item {
        std::string view;
        std::string text;
    };

    std::vector<Item> items = {};

    void drawItem(Point pos, Size _itemSize, std::string text)
    {
        draw.filledRect(pos, _itemSize, { colors.background });
        draw.textCentered({ pos.x + _itemSize.w / 2, pos.y + textTopMargin }, text, fontSize, { colors.font });
    }

    void setSizes()
    {
        itemSize = { (size.w - 2 * (arrowSize.w + 1)) / 3 - 1, size.h };
        textTopMargin = (arrowSize.h - fontSize) / 2;
        itemsFix = (size.w - (((itemSize.w + 1) * 3) + (arrowSize.w * 2) + 1));
    }

    void renderMain()
    {
        draw.filledRect(position, arrowSize, { colors.background });
        draw.lines({ { position.x + arrowSize.w - 3, position.y + 1 },
                       { position.x + 3, position.y + (int)(arrowSize.h * 0.5) },
                       { position.x + arrowSize.w - 3, position.y + arrowSize.h - 2 } },
            { colors.font });

        draw.filledRect({ position.x + size.w - arrowSize.w, position.y }, arrowSize, { colors.background });
        draw.lines({ { position.x + size.w - arrowSize.w + 3, position.y + 1 },
                       { position.x + size.w - 3, position.y + (int)(arrowSize.h * 0.5) },
                       { position.x + size.w - arrowSize.w + 3, position.y + arrowSize.h - 2 } },
            { colors.font });

        int xPos = position.x + (arrowSize.w + 1);
        for (int i = 0; i < 3; i++) {
            // Because width is not always exactly dividable by 3 pixelwise
            // we need to add some fix in the middle item
            Size _itemSize = { itemSize.w + (i == 1 ? itemsFix : 0), itemSize.h };
            drawItem({ xPos, position.y },
                _itemSize,
                items[(i + itemPos) % items.size()].text);
            xPos += _itemSize.w + 1;
        }
    }

    void renderSub()
    {
        for (int row = 0; row < 2; row++) {
            for (int col = 0; col < 5; col++) {
                Point pos = { position.x + col * (subItemSize.w + 1), position.y + row * subItemSize.h };
                draw.filledRect(pos, subItemSize, { colors.background });
                draw.textCentered({ pos.x + subItemSize.w / 2, pos.y }, "track" + std::to_string(row * 5 + col + 1), 9, { colors.font });
            }
        }
    }

    void handleButton(int8_t id, int8_t state)
    {
        // printf("handleButton: %d %d\n", id, state);
        showSub = state == 1;
        clear();
        renderNext();
    }

    struct Colors {
        Color background;
        Color font;
    } colors;

public:
    NavBarComponent(ComponentInterface::Props props)
        : Component(props)
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            /*md **Keyboard actions**: */
            /*md - `next` to navigate to the next items. */
            if (action == "next") {
                func = [&](KeypadLayout::KeyMap& keymap) { if (keymap.pressedTime != -1) { itemPos++; renderNext(); } };
            }
            /*md - `prev` to navigate to the previous items. */
            if (action == "prev") {
                func = [&](KeypadLayout::KeyMap& keymap) { if (keymap.pressedTime != -1) { itemPos--; renderNext(); } };
            }

            /*md - `btnLeft` to select left item. */
            if (action == "btnLeft") {
                func = [&](KeypadLayout::KeyMap& keymap) { handleButton(0, keymap.pressedTime != -1); };
            }

            /*md - `btnMiddle` to select middle item. */
            if (action == "btnMiddle") {
                func = [&](KeypadLayout::KeyMap& keymap) { handleButton(1, keymap.pressedTime != -1); };
            }

            /*md - `btnRight` to select right item. */
            if (action == "btnRight") {
                func = [&](KeypadLayout::KeyMap& keymap) { handleButton(2, keymap.pressedTime != -1); };
            }
            return func;
        })
    {
        colors.background = styles.colors.background;
        colors.font = { 0x80, 0x80, 0x80, 255 };
        arrowSize = { size.h, size.h };
        setSizes();
        subItemSize = { (size.w - 4) / 5, size.h / 2 };
        subItemsFix = (size.w - (((subItemSize.w + 1) * 5) - 1));
    }

    void render()
    {
        if (showSub) {
            renderSub();
        } else {
            renderMain();
        }
    }

    bool config(char* key, char* value)
    {
        /*md - `BACKGROUND: #333333` set the background color and activate background drawing. */
        if (strcmp(key, "BACKGROUND") == 0) {
            colors.background = draw.getColor(value);
            return true;
        }

        /*md - `FONT_COLOR: #ffffff` set the font color. */
        if (strcmp(key, "FONT_COLOR") == 0) {
            colors.font = draw.getColor(value);
            return true;
        }

        /*md - `FONT_SIZE: 10` set the font size. */
        if (strcmp(key, "FONT_SIZE") == 0) {
            fontSize = atoi(value);
            return true;
        }

        /*md - `ARROW_WIDTH: 10` set the arrow size. */
        if (strcmp(key, "ARROW_WIDTH") == 0) {
            arrowSize.w = atoi(value);
            setSizes();
            return true;
        }

        /*md - `ITEM: view label` add an item. */
        if (strcmp(key, "ITEM") == 0) {
            Item item;
            item.view = strtok(value, " ");
            item.text = strtok(NULL, "");
            items.push_back(item);
            return true;
        }

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
