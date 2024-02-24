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

```cpp
    .colors = {
        .background = {0x21, 0x25, 0x2b, 255},  // #21252b
        .overlay = {0xd1, 0xe3, 0xff, 0x1e},    // #d1e3ff1e
        .on = {0x00, 0xb3, 0x00, 255},          // #00b300
        .white = {0xff, 0xff, 0xff, 255},       // #ffffff
        .blue = {0xad, 0xcd, 0xff, 255},        // #adcdff
        .red = {0xff, 0x8d, 0x99, 255},         // #ff8d99
    },
```

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