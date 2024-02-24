#ifndef _UI_CONFIG_H_
#define _UI_CONFIG_H_

#include "controllers.h"
#include "event.h"
#include "host.h"
#include "styles.h"
#include "viewManager.h"

#include "helpers/configPlugin.h"

void uiScriptCallback(char* key, char* value, const char* filename)
{
    if (strcmp(key, "print") == 0) {
        printf(">> LOG: %s\n", value);
    } else if (strcmp(key, "LOAD_HOST") == 0) {
        std::string configPath = strtok(value, " ");
        char *pluginConfig = strtok(NULL, " ");
        if (!loadHost(getFullpath(value, filename), pluginConfig)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load host");
        }
    } else if (strcmp(key, "SCREEN") == 0) {
        styles.screen.w = atoi(strtok(value, " "));
        styles.screen.h = atoi(strtok(NULL, " "));
    } else if (strcmp(key, "PLUGIN_CONTROLLER") == 0) {
        loadPluginController(value, filename);
        /*md
### SET_COLOR

`SET_COLOR` give the possibility to customize the pre-defined color for the UI. To change a color, use `SET_COLOR: name_of_color #xxxxxx`.

```coffee
SET_COLOR: overlay #00FFFF
```

In this example, we change the `overlay` color to `#00FFFF`.

<code style="color: red">#ff8d99 red</code>

- `#21252b` background
- `#00FFFF` overlay
- `#00b300` on
- `#ffffff` white
- `#adcdff` blue
- `#ff8d99` red

> This list might be outdated, to get the list of existing colors, look at `./styles.h`
        */
    } else if (strcmp(key, "SET_COLOR") == 0) {
        char* name = strtok(value, " ");
        char* color = strtok(NULL, " ");
        ViewManager::get().draw.setColor(name, color);
    } else if (strcmp(key, "LOAD_CONFIG_PLUGIN") == 0) {
        char* scriptPath = strtok(value, " ");
        char* plugin = strtok(NULL, " ");
        loadConfigPlugin(scriptPath, plugin, uiScriptCallback);
    } else if (pluginControllerConfig(key, value) || EventHandler::get().config(key, value)) {
        return;
    } else {
        ViewManager::get().config(key, value, filename);
    }
}

void loadUiConfig(const char* scriptPath, const char* pluginPath)
{
    loadConfigPlugin(pluginPath, scriptPath, uiScriptCallback);
}

#endif