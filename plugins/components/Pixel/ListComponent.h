#pragma once

#include "plugins/components/base/Icon.h"
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
    Icon icon;

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

            if (keypadCustomAction) {
                func = keypadCustomAction(action);
            }

            return func;
        })
        ,
        icon(props.view->draw), bgColor(styles.colors.background), textColor(styles.colors.text), itemBackground(lighten(styles.colors.background, 0.5)), selectionColor(styles.colors.primary)
    {
        /*md md_config:List */
        nlohmann::json config = props.config;

        /*md   // The list of items to add in the list. */
        /*md   items={["item1", "item2", "item3"]} */
        if (config.contains("items") && config["items"].is_array()) {
            for (int i = 0; i < config["items"].size(); i++) {
                items.push_back({ config["items"][i].get<std::string>() });
            }
        }

        /*md   // The audio plugin to get control on. */
        /*md   audioPlugin="audio_plugin_name" */
        if (config.contains("audioPlugin")) {
            plugin = &getPlugin(config["audioPlugin"].get<std::string>().c_str(), track);
        }

        /*md   // Set the background color of the component. */
        /*md   bgColor="#000000" */
        bgColor = draw.getColor(config["bgColor"], bgColor);

        /*md   // Set the color of the text. */
        /*md   textColor="#ffffff" */
        textColor = draw.getColor(config["textColor"], textColor);

        /*md   // Set the color of the selection. */
        /*md   selectionColor="#ffffff" */
        selectionColor = draw.getColor(config["selectionColor"], selectionColor);

        /*md   // Set the color of the item background. */
        /*md   itemBackground="#ffffff" */
        itemBackground = draw.getColor(config["itemBackground"], itemBackground);

        /*md md_config_end */
    }

    virtual void renderItem(int y, int itemIndex)
    {
        draw.text({ relativePosition.x + 8, y + 4 }, items[itemIndex].text, 8, { textColor });
    }

    virtual void renderStart() { }
    virtual void renderEnd() { }

    void render()
    {
        if (updatePosition()) {
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
    }
};
