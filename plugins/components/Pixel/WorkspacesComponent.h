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

    void initItems() {
        items.clear();
        std::vector<std::filesystem::path> list = getDirectoryList(workspaceFolder, { .skipFiles = true });
        for (std::filesystem::path path : list) {
            items.push_back({ path.filename().string() });
        }
    }

    WorkspacesComponent(ComponentInterface::Props props)
        : ListComponent(props)
    {
        initItems();
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        if (ListComponent::config(key, value)) {
            return true;
        }

        /*md - `WORKSPACE_FOLDER: workspaceFolder` to set workspace folder. By default it is `workspaces`.*/
        if (strcmp(key, "WORKSPACE_FOLDER") == 0) {
            workspaceFolder = value;
            initItems();
            return true;
        }

        return false;
    }
};

#endif
