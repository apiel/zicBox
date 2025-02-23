#ifndef _CONTROLLERS_H_
#define _CONTROLLERS_H_

#include "controllerList.h"
#include "host.h"
#include "log.h"
#include "plugins/controllers/controllerInterface.h"
#include "viewManager.h"

#include <dlfcn.h>

ControllerInterface* lastPluginControllerInstance = NULL;

void encoderHandler(int id, int8_t direction, uint32_t tick)
{
    ViewManager::get().view->onEncoder(id, direction, tick);
}

void keyHandler(uint16_t id, int key, int8_t state)
{
    ViewManager::get().view->onKey(id, key, state);
}

uint16_t controllerId = 1;
ControllerInterface::Props controllerProps = { encoderHandler, keyHandler };

void loadPluginController(char* value, const char* filename)
{
    Controller plugin;
    strcpy(plugin.name, strtok(value, " "));
    char* path = strtok(NULL, " ");

    void* handle = dlopen(getFullpath(path, filename).c_str(), RTLD_LAZY);

    if (!handle) {
        logError("Cannot open controller library %s [%s]: %s\n", path, filename, dlerror());
        return;
    }

    dlerror();
    void* allocator = dlsym(handle, "allocator");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        logError("Cannot load symbol: %s\n", dlsym_error);
        dlclose(handle);
        return;
    }

    plugin.instance = ((ControllerInterface * (*)(ControllerInterface::Props & props, uint16_t id)) allocator)(controllerProps, controllerId++);
    lastPluginControllerInstance = plugin.instance;
    logDebug("plugin interface loaded: %s\n", path);

    controllers.push_back(plugin);
}

bool pluginControllerConfig(char* key, char* value)
{
    if (lastPluginControllerInstance) {
        // return lastPluginControllerInstance->config(key, value);
    }
    return false;
}

#endif
