#ifndef _UI_PIXEL_COMPONENT_WORKSPACES_H_
#define _UI_PIXEL_COMPONENT_WORKSPACES_H_

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

    Color badgeColor;

    void initItems()
    {
        items.clear();
        std::vector<std::filesystem::path> list = getDirectoryList(workspaceFolder, { .skipFiles = true });
        for (std::filesystem::path path : list) {
            items.push_back({ path.filename().string() });
        }
    }

    WorkspacesComponent(ComponentInterface::Props props)
        : ListComponent(props)
        , badgeColor({ 39, 128, 39 }) // rgb(39, 128, 39)
    {
        initItems();
    }

    void renderItem(int y, int itemIndex) override
    {
        draw.text({ relativePosition.x + 8, y + 4 }, items[itemIndex].text, 8, { textColor });
        if (currentWorkspaceName != NULL && items[itemIndex].text == *currentWorkspaceName) {
            Point pos = { relativePosition.x + size.w - 70, y + 3 };
            draw.rect(pos, { 10, 10 }, { badgeColor });
            draw.filledRect({ pos.x + 2, pos.y + 2 }, { 6, 7 }, { badgeColor });
            draw.text({ pos.x + 14, pos.y + 2 }, "active", 8, { badgeColor });
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        /*md - `PLUGIN: plugin` is the plugin to use to make action on the list. */
        if (strcmp(key, "PLUGIN") == 0) {
            plugin = &getPlugin(strtok(value, " "), track);
            uint8_t dataId = plugin->getDataId("CURRENT_WORKSPACE");
            currentWorkspaceName = (std::string*)plugin->data(dataId);
            return true;
        }

        /*md - `WORKSPACE_FOLDER: workspaceFolder` to set workspace folder. By default it is `workspaces`.*/
        if (strcmp(key, "WORKSPACE_FOLDER") == 0) {
            workspaceFolder = value;
            initItems();
            return true;
        }

        /*md - `BADGE_COLOR: badgeColor` to set badge color. By default it is `#23a123`.*/
        if (strcmp(key, "BADGE_COLOR") == 0) {
            badgeColor = draw.getColor(value);
            return true;
        }

        if (ListComponent::config(key, value)) {
            return true;
        }

        return false;
    }
};

#endif
