/** Description:
This code defines an interactive user interface element called the `KeyInfoBarComponent`. Its primary function is to serve as an on-screen informational display, typically used to show available actions or shortcuts associated with a physical or virtual keypad (keyboard) layout.

### Structure and Display

The component presents information as a grid of distinct "items" or buttons. The arrangement (number of columns and rows) is automatically calculated based on the total space available and the configuration settings. Each item displays a label or a small icon.

The appearance is highly customizable:
*   Items use specific background and font colors.
*   When a user interacts with an item, it changes color to visually indicate that it is the "active" selection.

### How It Works

1.  **Layout Calculation:** The component determines the size of each item so that they fit neatly into the available display area according to the specified number of columns.
2.  **Rendering:** It draws each item as a colored block, centering the text and icons within its boundaries. Only items that have changed are redrawn to ensure efficient updates.
3.  **Input Handling:** It uses a specialized internal system to monitor user input (key presses). When a key mapped to an item is pressed, the component handles the selection.
4.  **Dynamic Menus:** A key feature is the ability for items to have "children." If a selected item has children, the component temporarily replaces the displayed text of all items with these sub-options, acting like a quick, temporary menu for deeper selection.

In essence, the `KeyInfoBarComponent` acts as a responsive and configurable information panel that guides the user by showing the state and available operations of the system's input device.

sha: 3d4b5cedaba89aebe4db05b45e2a72700c40220d82b26c5ac623f3070bf74494 
*/
#ifndef _UI_COMPONENT_KEY_INFO_BAR_H_
#define _UI_COMPONENT_KEY_INFO_BAR_H_

#include "../base/KeypadLayout.h"
#include "../utils/color.h"
#include "../base/Icon.h"
#include "../component.h"

#include <cmath>
#include <string>
#include <vector>

/*md
## SDL KeyInfoBar

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
    int iconFix = 1;
    int iconMarginTop = 1;

    Icon icon;

    KeypadLayout keypadLayout;
    int8_t activeItem = -1;

    struct Item {
        std::string text = "                                               ";
        std::string title = "                                               ";
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

        draw.filledRect(pos, itemSize, { background });

        Point textPos = { pos.x + textLeftMargin, pos.y + textTopMargin };
        if (!icon.render(items[index].text, { textPos.x, textPos.y + iconFix + iconMarginTop }, fontSize - iconFix, fontColor, Icon::CENTER)) {
            draw.textCentered(textPos, items[index].text, fontSize, { fontColor });
        }
    }

    void handleButton(int8_t id, int8_t state)
    {
        items[activeItem].needUpdate = true;
        // TODO work on key combination...
        if (state == 0) {
            if (activeItem == id) {
                if (items[activeItem].children.size()) {
                    for (int i = 0; i < items[activeItem].children.size(); i++) {
                        items[i].text = items[i].title;
                        // printf("set %d to %s %s\n", i, items[i].text.c_str(), items[i].title.c_str());
                        items[i].needUpdate = true;
                    }
                }
                activeItem = -1;
            }
        } else if (activeItem == -1) {
            activeItem = id;
            if (items[activeItem].children.size()) {
                for (int i = 0; i < items[activeItem].children.size(); i++) {
                    // printf("was %d to %s %s\n", i, items[i].text.c_str(), items[i].title.c_str());
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
    KeyInfoBarComponent(ComponentInterface::Props props)
        : Component(props)
        , icon(props.view->draw)
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            /*md **Keyboard actions**: */
            /*md - `item` to select item. */
            if (action.rfind("item:") == 0) {
                int* id = new int(atoi(action.substr(5).c_str()));
                func = [&, id](KeypadLayout::KeyMap& keymap) { handleButton(*id, keymap.pressedTime != -1); };
            }
            return func;
        })
    {
        colors.background = lighten(styles.colors.background, 0.2);
        colors.font = { 0x80, 0x80, 0x80, 255 };
        colors.activeBackground = lighten(colors.background, 0.4);
        colors.activeFont = lighten(colors.font, 0.4);
        // resize(); // No need to call when empty
    }

    void resize()
    {
        if (items.size()) {
            itemRowCount = std::ceil(items.size() / (float)itemColumnCount);
            itemSize = { (size.w / itemColumnCount) - 1, (size.h / itemRowCount) - 1 };
            textTopMargin = (int)((itemSize.h - fontSize) / 2.0f) - 1;
            // printf("textTopMargin: %d - %d = %f = %d\n", itemSize.h, fontSize, ((itemSize.h - fontSize) / 2.0f), textTopMargin);
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

            // if fontsize impaire remove 1 pixel else 0
            iconFix = fontSize % 2;
            return true;
        }

        /*md - `ICON_FIX: 1` remove 1 pixel to icon size, `ICON_FIX: 2` remove 2 pixel... */
        if (strcmp(key, "ICON_FIX") == 0) {
            iconFix = atoi(value);
            return true;
        }

        /*md - `ICON_MARGIN_TOP: 1` set the icon top margin. */
        if (strcmp(key, "ICON_MARGIN_TOP") == 0) {
            iconMarginTop = atoi(value);
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
            item.title = value;
            item.text = item.title;
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
