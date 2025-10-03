#pragma once

#include "./ListComponent.h"

#include "helpers/fs/directoryList.h"

#include <string>
#include <vector>

/*md
## Workspaces

Workspaces components show the list of available workspaces.
*/

class WorkspacesComponent : public ListComponent {
public:
    std::string workspaceFolder = "workspaces";
    std::string* currentWorkspaceName = NULL;
    std::string* repositoryFolder = NULL;
    int* refreshState = NULL;
    int currentRefreshState = 0;

    Color badgeColor;
    Color errorColor;

    enum Error {
        NONE = 0,
        DELETE = 1,
    };
    uint8_t error = Error::NONE;

    std::string getFolder()
    {
        return *repositoryFolder + "/" + workspaceFolder;
    }

    void initItems()
    {
        items.clear();
        std::vector<std::filesystem::path> list = getDirectoryList(getFolder(), { .skipFiles = true });
        for (std::filesystem::path path : list) {
            items.push_back({ path.filename().string() });
        }
    }

    WorkspacesComponent(ComponentInterface::Props props)
        : ListComponent(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == ".delete") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (currentWorkspaceName != NULL && items[selection].text == *currentWorkspaceName) {
                            error = Error::DELETE;
                            renderNext();
                        } else {
                            uint8_t dataId = plugin->getDataId("DELETE_WORKSPACE");
                            plugin->data(dataId, &items[selection].text);
                            initItems();
                            renderNext();
                        }
                    }
                };
            }
            return func;
        })
        , badgeColor(rgb(39, 128, 39))
        , errorColor(rgb(173, 99, 99))
    {
        /*md md_config:Workspaces */
        nlohmann::json& config = props.config;

        /// The audio plugin to load serialized data.
        plugin = getPluginPtr(config, "audioPlugin", track); //eg: "audio_plugin_name"

        currentWorkspaceName = (std::string*)plugin->data(plugin->getDataId(config.value("currentWorkspaceDataId", "CURRENT_WORKSPACE"))); //eg: "CURRENT_WORKSPACE"
        repositoryFolder = (std::string*)plugin->data(plugin->getDataId(config.value("repositoryFolderDataId", "REPOSITORY_FOLDER"))); //eg: "REPOSITORY_FOLDER"

        refreshState = (int*)plugin->data(plugin->getDataId(config.value("refreshStateDataId", "CREATE_WORKSPACE"))); //eg: "CREATE_WORKSPACE"
        currentRefreshState = *refreshState;

        /// The background color of the text.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        /// Color the active workspace badge.
        badgeColor = draw.getColor(config["badgeColor"], badgeColor); //eg: "#23a123"

        /// Color of the error message.
        errorColor = draw.getColor(config["errorColor"], errorColor); //eg: "#ab6363"

        /*md md_config_end */

        initItems();
    }

    void renderItem(int y, int itemIndex) override
    {
        draw.text({ relativePosition.x + 8, y }, items[itemIndex].text, 12, { textColor });
        if (currentWorkspaceName != NULL && items[itemIndex].text == *currentWorkspaceName) {
            Point pos = { relativePosition.x + size.w - 70, y + 3 };
            draw.rect(pos, { 10, 10 }, { badgeColor });
            draw.filledRect({ pos.x + 2, pos.y + 2 }, { 6, 7 }, { badgeColor });
            draw.text({ pos.x + 14, pos.y - 2 }, "active", 12, { badgeColor });
        }
    }

    void renderStart() override
    {
        if (currentRefreshState != *refreshState) {
            currentRefreshState = *refreshState;
            initItems();
        }
    }

    void renderEnd() override
    {
        if (error == Error::DELETE) {
            Point pos = { relativePosition.x + 20, relativePosition.y + 20 };
            draw.filledRect(pos, { size.w - 40, 30 }, { errorColor });
            draw.textCentered({ relativePosition.x + (int)(size.w / 2), pos.y + 2 }, "Cannot delete", 12, { textColor });
            draw.textCentered({ relativePosition.x + (int)(size.w / 2), pos.y + 14 }, "active workspace.", 12, { textColor });
            error = Error::NONE;
        }
    }
};
