/** Description:
This C++ header defines the `WorkspacesComponent`, a specialized user interface element designed to manage and display a dynamic list of available project folders, or "workspaces," within an application environment.

This component functions essentially as a controlled list viewer. Its primary mechanism involves reading a specific directory on the file system to identify all subfolders, treating each subfolder name as an available workspace.

The component is highly integrated with the main application system (managed through a "plugin" structure). It retrieves crucial information, such as the exact location of the workspace folder, the name of the currently active workspace, and status signals that trigger an automatic refresh of the list (for instance, when a new workspace is created elsewhere).

Visually, the component provides excellent feedback: it highlights the active workspace with a specific colored badge. Furthermore, it manages user interaction, specifically the ability to delete workspaces. Crucially, it prevents deletion attempts on the workspace currently in use, displaying a temporary error message if this protection mechanism is triggered.

In summary, this component handles the data loading, display, visual differentiation, synchronization, and safe management of project contexts for the user interface.

sha: 91b351381d79120e7f9547279f3c2872fa3ab6d00985a3889bd840c3e3eafc7c 
*/
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
    std::string* workspaceFolder = NULL;
    std::string* currentWorkspaceName = NULL;
    int* refreshState = NULL;
    int currentRefreshState = 0;

    Color badgeColor;
    Color errorColor;

    enum Error {
        NONE = 0,
        DELETE = 1,
    };
    uint8_t error = Error::NONE;

    void initItems()
    {
        if (workspaceFolder != NULL) {
            items.clear();
            std::vector<std::filesystem::path> list = getDirectoryList(*workspaceFolder, { .skipFiles = true });
            for (std::filesystem::path path : list) {
                items.push_back({ path.filename().string() });
            }
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

        workspaceFolder = (std::string*)plugin->data(plugin->getDataId(config.value("workspaceFolderDataId", "WORKSPACE_FOLDER")));
        currentWorkspaceName = (std::string*)plugin->data(plugin->getDataId(config.value("currentWorkspaceDataId", "CURRENT_WORKSPACE"))); //eg: "CURRENT_WORKSPACE"

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
