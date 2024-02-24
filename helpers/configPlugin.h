#ifndef _CONFIG_PLUGIN_H
#define _CONFIG_PLUGIN_H

#include "getFullpath.h"

#include <dlfcn.h>
#include <stdio.h>

void instantiateConfigPlugin(const char* pluginPath, const char* scriptPath, void (*callback)(char* command, char* params, const char* scriptPath))
{
    void* handle = dlopen(pluginPath, RTLD_LAZY);
    if (!handle) {
        printf("Cannot open library: %s\n", dlerror());
        return;
    }

    dlerror();

    void (*configParser)(const char* scriptPath, void (*callback)(char* command, char* params, const char* scriptPath));
    configParser = (void (*)(const char* scriptPath, void (*callback)(char* command, char* params, const char* scriptPath)))dlsym(handle, "config");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        printf("Cannot load symbol: %s\n", dlsym_error);
        dlclose(handle);
        return;
    }

    configParser(scriptPath, callback);
    dlclose(handle);
}

void loadConfigPlugin(const char* pluginPath, const char* scriptPath, void (*callback)(char* command, char* params, const char* scriptPath))
{
    if (!pluginPath) {
        const char* extension = strrchr(scriptPath, '.');

        if (strcmp(extension, ".lua") == 0) {
            pluginPath = "lua";
        } else {
            pluginPath = "dustscript";
        }
    }

    if (strcmp(pluginPath, "dustscript") == 0) {
        instantiateConfigPlugin("plugins/config/build/libzic_DustConfig.so", scriptPath, callback);
    } else if (strcmp(pluginPath, "lua") == 0) {
        instantiateConfigPlugin("plugins/config/build/libzic_LuaConfig.so", scriptPath, callback);
    } else {
        instantiateConfigPlugin(pluginPath, scriptPath, callback);
    }
}

#endif
