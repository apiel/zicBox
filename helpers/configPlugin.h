#ifndef _CONFIG_PLUGIN_H
#define _CONFIG_PLUGIN_H

#include "getFullpath.h"
#include "plugins/config/LuaConfig.h"

#include <dlfcn.h>
#include <stdio.h>
#include <string>
#include <vector>

// struct Var {
//     std::string key;
//     std::string value;
// };

void instantiateConfigPlugin(const char* pluginPath, std::string scriptPath, void (*callback)(char* command, char* params, const char* scriptPath, std::vector<Var> variables), std::vector<Var> variables)
{
    void* handle = dlopen(pluginPath, RTLD_LAZY);
    if (!handle) {
        printf("Cannot open config library %s [%s]: %s\n", pluginPath, scriptPath.c_str(), dlerror());
        return;
    }

    dlerror();

    void (*configParser)(std::string scriptPath, void (*callback)(char* command, char* params, const char* filename, std::vector<Var> variables), std::vector<Var> variables);
    configParser = (void (*)(std::string scriptPath, void (*callback)(char* command, char* params, const char* filename, std::vector<Var> variables), std::vector<Var> variables))dlsym(handle, "config");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        printf("Cannot load symbol: %s\n", dlsym_error);
        dlclose(handle);
        return;
    }

    configParser(scriptPath, callback, variables);
    dlclose(handle);
}

void loadConfigPlugin(const char* pluginPath, std::string scriptPath, void (*callback)(char* command, char* params, const char* scriptPath, std::vector<Var> variables), std::vector<Var> variables = {})
{
    if (!pluginPath) {
        pluginPath = "lua";
    }

    if (strcmp(pluginPath, "dustscript") == 0) {
#ifdef IS_RPI
        instantiateConfigPlugin("plugins/config/build/arm/libzic_DustConfig.so", scriptPath, callback, variables);
#else
        instantiateConfigPlugin("plugins/config/build/x86/libzic_DustConfig.so", scriptPath, callback, variables);
#endif
    } else if (strcmp(pluginPath, "lua") == 0) {
        luaConfig(scriptPath, callback, variables);
    } else {
        instantiateConfigPlugin(pluginPath, scriptPath, callback, variables);
    }
}

#endif
