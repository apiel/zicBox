#ifndef _CONFIG_PLUGIN_H
#define _CONFIG_PLUGIN_H

#include "getFullpath.h"

#include <dlfcn.h>
#include <stdio.h>

void loadConfigPlugin(char* path, const char* filename, void (*callback)(char* command, char* params, const char* filename))
{
    void* handle = dlopen(getFullpath(path, filename).c_str(), RTLD_LAZY);
    if (!handle) {
        printf("Cannot open library: %s\n", dlerror());
        return;
    }

    dlerror();

    double (*configParser)(const char* filename, void (*callback)(char* command, char* params, const char* filename));
    configParser = (double (*)(const char* filename, void (*callback)(char* command, char* params, const char* filename))) dlsym(handle, "config");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        printf("Cannot load symbol: %s\n", dlsym_error);
        dlclose(handle);
        return;
    }

    configParser(filename, callback);
    dlclose(handle);
}

#endif
