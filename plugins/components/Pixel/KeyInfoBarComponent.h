#ifndef _UI_PIXEL_COMPONENT_KEY_INFO_BAR_H_
#define _UI_PIXEL_COMPONENT_KEY_INFO_BAR_H_

#include "../base/Icon.h"
#include "../base/KeypadLayout.h"
#include "../component.h"

#include <cmath>
#include <string>
#include <vector>

/*md
## KeyInfoBar

KeyInfoBar components provide some information about the current kaypad layout.
*/

// Instead to have child elements, use same concept as group...
// presskey can set visibility of a given component

// KEYMAP in addition of key number, we could use engish keyboard value

class KeyInfoBarComponent : public Component {
protected:
    Icon icon;
    KeypadLayout keypadLayout;
    int8_t activeGroup = 0;

    Color bgColor;
    Color textColor;

    //                        { index, value}
    int16_t shiftVisibility[2] = { -1, -1 };

public:
    KeyInfoBarComponent(ComponentInterface::Props props)
        : Component(props)
        , icon(props.view->draw)
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
        , keypadLayout(this)
    {
    }

protected:
    struct Row {
        std::vector<std::string> text;
        int startX = 0;
        int width = 0;
    };

    std::vector<Row> rows;

    int renderRow(int y, Row& row)
    {
        int index = 0;
        for (auto text : row.text) {
            Point textPos = { relativePosition.x + row.startX + index * row.width, y };
            if (!icon.render(text, textPos, 8, {}, Icon::CENTER)) {
                draw.textCentered(textPos, text, 8);
            }
            index++;
        }
        return 12;
    }

    bool isVisible()
    {
        return shiftVisibility[0] == -1 || view->contextVar[shiftVisibility[0]] == shiftVisibility[1];
    }

public:
    void render()
    {
        if (isVisible() && updatePosition()) {
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
                row.text.push_back(text);
                text = strtok(NULL, " ");
            }
            row.width = size.w / row.text.size();
            row.startX = row.width * 0.5f;
            rows.push_back(row);
            return true;
        }

        /*md - `SHIFT_VISIBILITY: index value` is the index and value to make the component visible or not. */
        if (strcmp(key, "SHIFT_VISIBILITY") == 0) {
            shiftVisibility[0] = atoi(strtok(value, " "));
            shiftVisibility[1] = atoi(strtok(NULL, " "));
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

        return false;
    }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        if (isVisible()) {
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
        if (index == shiftVisibility[0]) {
            renderNext();
        }
    }
};

#endif
