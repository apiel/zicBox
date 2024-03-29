#ifndef _CONTROLLERS_H_
#define _CONTROLLERS_H_

#include "host.h"
#include "plugins/controllers/controllerInterface.h"
#include "viewManager.h"
#include "controllerList.h"

#include <dlfcn.h>

ControllerInterface* lastPluginControllerInstance = NULL;

void encoderHandler(int id, int8_t direction, uint32_t tick)
{
    ViewManager::get().onEncoder(id, direction, tick);
}

void keyHandler(uint16_t id, int key, int8_t state)
{
    ViewManager::get().onKey(id, key, state);
}

uint16_t controllerId = 1;
void loadPluginController(char* value, const char* filename)
{
    Controller plugin;
    strcpy(plugin.name, strtok(value, " "));
    char* path = strtok(NULL, " ");
    
    void* handle = dlopen(getFullpath(path, filename).c_str(), RTLD_LAZY);

    if (!handle) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot open controller library %s [%s]: %s\n", path, filename, dlerror());
        return;
    }

    dlerror();
    void* allocator = dlsym(handle, "allocator");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot load symbol: %s\n", dlsym_error);
        dlclose(handle);
        return;
    }

    ControllerInterface::Props props = { midiHandler, encoderHandler, keyHandler };
    plugin.instance = ((ControllerInterface * (*)(ControllerInterface::Props & props, uint16_t id)) allocator)(props, controllerId++);
    lastPluginControllerInstance = plugin.instance;
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "plugin interface loaded: %s\n", path);

    controllers.push_back(plugin);
}

bool pluginControllerConfig(char* key, char* value)
{
    if (lastPluginControllerInstance) {
        return lastPluginControllerInstance->config(key, value);
    }
    return false;
}

#endif
