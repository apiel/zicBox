#ifndef _CONFIG_PLUGIN_H
#define _CONFIG_PLUGIN_H

#include "getFullpath.h"
#include "plugins/config/DustConfig.h"

#include <dlfcn.h>
#include <stdio.h>
#include <string>
#include <vector>

// struct Var {
//     std::string key;
//     std::string value;
// };

void instantiateConfigPlugin(const char* pluginPath, std::string scriptPath, void (*callback)(char* command, char* params, const char* scriptPath), std::vector<Var> variables = {})
{
    void* handle = dlopen(pluginPath, RTLD_LAZY);
    if (!handle) {
        printf("Cannot open config library %s [%s]: %s\n", pluginPath, scriptPath.c_str(), dlerror());
        return;
    }

    dlerror();

    void (*configParser)(std::string scriptPath, void (*callback)(char* command, char* params, const char* filename), std::vector<Var> variables);
    configParser = (void (*)(std::string scriptPath, void (*callback)(char* command, char* params, const char* filename), std::vector<Var> variables))dlsym(handle, "config");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        printf("Cannot load symbol: %s\n", dlsym_error);
        dlclose(handle);
        return;
    }

    configParser(scriptPath, callback, variables);
    dlclose(handle);
}

void loadConfigPlugin(const char* pluginPath, std::string scriptPath, void (*callback)(char* command, char* params, const char* scriptPath), std::vector<Var> variables = {})
{
    if (!pluginPath) {
        const char* extension = strrchr(scriptPath.c_str(), '.');

        if (strcmp(extension, ".lua") == 0) {
            pluginPath = "lua";
        } else {
            pluginPath = "dustscript";
        }
    }

    if (strcmp(pluginPath, "dustscript") == 0) {
        dustConfig(scriptPath, callback, variables);
    } else if (strcmp(pluginPath, "lua") == 0) {
#ifdef IS_RPI
        instantiateConfigPlugin("plugins/config/build/arm/libzic_LuaConfig.so", scriptPath, callback, variables);
#else
        instantiateConfigPlugin("plugins/config/build/x86/libzic_LuaConfig.so", scriptPath, callback, variables);
#endif
    } else {
        instantiateConfigPlugin(pluginPath, scriptPath, callback, variables);
    }
}

#endif
