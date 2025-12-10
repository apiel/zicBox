/** Description:
This code defines a standard user interface element called the "List Component," primarily used to display a menu or a collection of items on a screen.

**Purpose and Functionality**

The List Component acts as a visual container for selectable data. It manages several items (usually strings of text) and controls their appearance and selection status.

**Key Features:**

1.  **Appearance Control:** It allows extensive customization through various color settings, including the background color of the list itself, the color of the unselected items, the text color, and a dedicated highlight color for the item currently chosen by the user.
2.  **Navigation:** It is designed to be interactive, responding to directional inputs (like Up and Down key presses). These inputs automatically update which item is highlighted in the list.
3.  **Efficient Display:** When drawing the list on the screen, it intelligently calculates how many items can fit in its designated space. If the list is longer than the screen space, it only renders the visible section, effectively managing a scrolling view.
4.  **System Integration:** A significant capability is its ability to communicate with external system parts, such as an Audio Plugin. When a user selects an item and confirms an action, the component can take the information from that selected item and pass it to the plugin, allowing the user interface choice to immediately affect the system's operational logic.

In short, the List Component is a versatile, navigable menu block that allows users to make choices and transmit those selections to the rest of the application.

sha: 63cd750c9e8c73dfb9ba69dcd9df6773a370fc5c3cfe9fdd2173892213956182 
*/
#pragma once

#include "plugins/components/component.h"
#include "plugins/components/utils/VisibilityContext.h"
#include "plugins/components/utils/VisibilityGroup.h"
#include "plugins/components/utils/color.h"

#include <string>
#include <vector>

/*md
## List

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/list.png" />

List components to display list of items.
*/

class ListComponent : public Component {
protected:
    Color bgColor;
    Color itemBackground;
    Color selectionColor;
    Color textColor;

    int selection = 0;

    // std::string redirectView = "";

    AudioPlugin* plugin = NULL;

    struct Item {
        std::string text;
        void* data = NULL;
    };

    std::vector<Item> items;

    int itemH = 16;

public:
    ListComponent(ComponentInterface::Props props, std::function<std::function<void(KeypadLayout::KeyMap& keymap)>(std::string action)> keypadCustomAction = nullptr)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;

            if (keypadCustomAction) {
                func = keypadCustomAction(action);
            }
            
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

            if (action.find(".data:") == 0) {
                if (plugin) {
                    uint8_t dataId = plugin->getDataId(action.substr(6));
                    func = [this, dataId](KeypadLayout::KeyMap& keymap) {
                        printf("data (%d): %s\n", dataId, items[selection].text.c_str());
                        if (KeypadLayout::isReleased(keymap)) {
                            plugin->data(dataId, &items[selection].text);
                            renderNext();
                        }
                    };
                }
            }

            if (action == ".setView") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        view->setView(items[selection].text);
                    }
                };
            }

            return func;
        }, true)
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
        , itemBackground(lighten(styles.colors.background, 0.5))
        , selectionColor(styles.colors.primary)
    {
        /*md md_config:List */
        nlohmann::json& config = props.config;

        /// The list of items to add in the list.
        if (config.contains("items") && config["items"].is_array()) { //eg: ["item1", "item2", "item3"]
            for (int i = 0; i < config["items"].size(); i++) {
                items.push_back({ config["items"][i].get<std::string>() });
            }
        }

        /// The audio plugin to get control on.
        if (config.contains("audioPlugin")) {
            plugin = &getPlugin(config["audioPlugin"].get<std::string>(), track); //eg: "audio_plugin_name"
        }

        /// Set the background color of the component.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        /// Set the color of the text.
        textColor = draw.getColor(config["textColor"], textColor); //eg: "#ffffff"

        /// Set the color of the selection.
        selectionColor = draw.getColor(config["selectionColor"], selectionColor); //eg: "#ffffff"

        /// Set the color of the item background.
        itemBackground = draw.getColor(config["itemBackground"], itemBackground); //eg: "#ffffff"

        keypadLayout.init(config);
        /*md md_config_end */
    }

    virtual void renderItem(int y, int itemIndex)
    {
        draw.text({ relativePosition.x + 8, y }, items[itemIndex].text, 12, { textColor });
    }

    virtual void renderStart() { }
    virtual void renderEnd() { }

    void render()
    {
        draw.filledRect(relativePosition, size, { bgColor });
        renderStart();

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
            renderItem(y, k);
            y += itemH + 2;
        }
        renderEnd();
    }
};
