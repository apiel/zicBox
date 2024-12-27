#ifndef _UI_PIXEL_COMPONENT_TEXT_GRID_H_
#define _UI_PIXEL_COMPONENT_TEXT_GRID_H_

#include "plugins/components/base/Icon.h"
#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/VisibilityContext.h"
#include "plugins/components/utils/color.h"

#include <cmath>
#include <string>
#include <vector>

/*md
## TextGrid

TextGrid components to display text in a table.
*/

class TextGridComponent : public Component {
protected:
    Icon icon;
    KeypadLayout keypadLayout;
    int8_t activeGroup = 0;

    Color bgColor;
    Color itemBackground;
    Color textColor;
    Color textColor2;

    VisibilityContext visibility;

public:
    TextGridComponent(ComponentInterface::Props props)
        : Component(props)
        , icon(props.view->draw)
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
        , textColor2(darken(styles.colors.text, 0.5))
        , itemBackground(lighten(styles.colors.background, 0.5))
        , keypadLayout(this)
    {
    }

protected:
    struct Item {
        std::string text;
        bool activeBackground = false;
        bool secondColor = false;
    };

    struct Row {
        std::vector<Item> items;
        int startX = 0;
        int width = 0;
    };

    std::vector<Row> rows;

    int renderRow(int y, Row& row)
    {
        int index = 0;
        int h = 12;
        for (auto item : row.items) {
            Point textPos = { relativePosition.x + row.startX + index * row.width, y };
            if (item.activeBackground) {
                draw.filledRect({ textPos.x - row.startX, textPos.y }, { row.width, h - 1 }, { itemBackground });
            }
            Color color = item.secondColor ? textColor2 : textColor;
            if (!icon.render(item.text, textPos, 8, { color }, Icon::CENTER)) {
                draw.textCentered(textPos, item.text, 8, { color });
            }
            index++;
        }
        return h;
    }

public:
    void render()
    {
        if (visibility.visible && updatePosition()) {
            draw.filledRect(relativePosition, size, { bgColor });
            int y = relativePosition.y;
            for (auto row : rows) {
                y += renderRow(y, row);
            }
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        if (keypadLayout.config(key, value)) {
            return true;
        }

        /*md - `ROW: text text text text text` is the text to add in row. */
        if (strcmp(key, "ROW") == 0) {
            Row row;
            char* text = strtok(value, " ");
            while (text != NULL) {
                Item item = { text, false };
                if (item.text[0] == '!') {
                    item.activeBackground = true;
                    item.text = item.text.substr(1);
                } else if (item.text[0] == '^') {
                    item.secondColor = true;
                    item.text = item.text.substr(1);
                }
                row.items.push_back(item);
                text = strtok(NULL, " ");
            }
            row.width = size.w / row.items.size();
            row.startX = row.width * 0.5f;
            rows.push_back(row);
            return true;
        }

        /*md - `VISIBILITY_CONTEXT: index SHOW_WHEN/SHOW_WHEN_NOT/SHOW_WHEN_OVER/SHOW_WHEN_UNDER value` the context index to show/hide the components for a given value. */
        if (visibility.config(key, value)) {
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is the color of the background. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(value);
            return true;
        }

        /*md - `TEXT_COLOR: color` is the color of the text. */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            textColor = draw.getColor(value);
            return true;
        }

        /*md - `TEXT_COLOR2: color` is the color of the text. */
        if (strcmp(key, "TEXT_COLOR2") == 0) {
            textColor2 = draw.getColor(value);
            return true;
        }

        /*md - `ITEM_BACKGROUND: color` is the color of the item background. */
        if (strcmp(key, "ITEM_BACKGROUND") == 0) {
            itemBackground = draw.getColor(value);
            return true;
        }

        return false;
    }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        if (visibility.visible) {
            keypadLayout.onKey(id, key, state, now);
        }
    }

    void onGroupChanged(int8_t index) override
    {
        activeGroup = index;
    }

    void initView(uint16_t counter)
    {
        keypadLayout.renderKeypadColor();
    }

    void onContext(uint8_t index, float value) override
    {
        if (visibility.onContext(index, value)) {
            renderNext();
        }
    }
};

#endif
