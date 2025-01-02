#ifndef _UI_PIXEL_COMPONENT_LIST_H_
#define _UI_PIXEL_COMPONENT_LIST_H_

#include "plugins/components/base/Icon.h"
#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/VisibilityContext.h"
#include "plugins/components/utils/VisibilityGroup.h"
#include "plugins/components/utils/color.h"

#include <string>
#include <vector>

/*md
## List

List components to display list of items.
*/

class ListComponent : public Component {
protected:
    Icon icon;
    KeypadLayout keypadLayout;

    Color bgColor;
    Color itemBackground;
    Color selectionColor;
    Color textColor;

    int selection = 0;

    std::string redirectView = "";

    std::string value = "";

    AudioPlugin* plugin = NULL;

    std::vector<std::string> items = {
        "default",
        "tekno 23",
        "darkpsy",
        "live 23",
        "project 1",
        "project 2",
        "project 3",
        "project 4",
        "project 5",
        "project 6",
        "project 7",
        "project 8",
        "project 9",
        "project 10",
        "project 11",
        "project 12",
        "project 13",
        "project 14",
        "project 15",
        "project 16",
        "project 17",
        "project 18",
        "project 19",
        "project 20"
    };

    int itemH = 16;

public:
    ListComponent(ComponentInterface::Props props)
        : Component(props)
        , icon(props.view->draw)
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
        , itemBackground(lighten(styles.colors.background, 0.5))
        , selectionColor(styles.colors.primary)
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == ".up") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (selection > 0) {
                            selection--;
                            renderNext();
                        }
                    }
                };
            }
            if (action == ".down") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (selection < items.size() - 1) {
                            selection++;
                            renderNext();
                        }
                    }
                };
            }

            return func;
        })
    {
    }

    void render()
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { bgColor });
            int y = relativePosition.y;
            int yEnd = y + size.h;
            int itemW = size.w;

            int countOfVisibleItems = (yEnd - y) / (itemH + 2);
            int start = 0;
            if (selection > countOfVisibleItems) {
                start = selection - countOfVisibleItems;
            }

            for (int k = start; k < items.size() && y < yEnd; k++) {
                draw.filledRect({ relativePosition.x, y }, { itemW, itemH }, { k == selection ? selectionColor : itemBackground });
                draw.text({ relativePosition.x + 8, y + 4 }, items[k], 8, { textColor });
                y += itemH + 2;
            }
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        if (keypadLayout.config(key, value)) {
            return true;
        }

        /*md - `REDIRECT_VIEW: viewName` is the view to return when the edit is finished. */
        if (strcmp(key, "REDIRECT_VIEW") == 0) {
            redirectView = value;
            return true;
        }

        // /*md - `DONE_DATA: plugin data_id` is the data id to return when the edit is done. */
        // if (strcmp(key, "DONE_DATA") == 0) {
        //     plugin = &getPlugin(strtok(value, " "), track);
        //     dataId = plugin->getDataId(strtok(NULL, " "));
        //     return true;
        // }

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

        /*md - `SELECTION_COLOR: color` is the color of the selection. */
        if (strcmp(key, "SELECTION_COLOR") == 0) {
            selectionColor = draw.getColor(value);
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
        keypadLayout.onKey(id, key, state, now);
    }
};

#endif
