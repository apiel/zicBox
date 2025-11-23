/** Description:
This code serves as the core management system for handling physical hardware inputs and integrating external software components (plugins) into the application. It is designed to be highly flexible, allowing the system to easily support different hardware devices like buttons or rotary knobs.

The main purpose of this header is managing dynamic system functionality. It allows the core program to load and use specialized "controller" modules without needing to be recompiled every time a new device is supported.

Two essential functions (`encoderHandler` and `keyHandler`) are defined to standardize input processing. When a physical action occurs (a knob is turned or a button is pressed), these functions capture the event and immediately pass it along to the currently displayed screen, ensuring the user interface responds instantly.

The crucial process, `loadPluginController`, handles system expansion. This function dynamically loads an external plugin file, verifies its integrity, initializes the new controller component with a unique ID, and provides it with the standard input communication channels. This architectural design makes the system highly modular, allowing diverse hardware controllers to be swapped in or out easily while logging any errors transparently.

sha: 4a5d1f7d012799983af1a216dbd1407629ed23178d18c63bedea783a5cda7631 
*/
#ifndef _CONTROLLERS_H_
#define _CONTROLLERS_H_

#include "controllerList.h"
#include "host.h"
#include "log.h"
#include "plugins/controllers/controllerInterface.h"
#include "viewManager.h"

#include <dlfcn.h>

ControllerInterface* lastPluginControllerInstance = NULL;

void encoderHandler(int8_t id, int8_t direction, uint32_t tick)
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

    void* handle = dlopen(path, RTLD_LAZY);

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

    // TODO pass the controller config here...

    plugin.instance = ((ControllerInterface * (*)(ControllerInterface::Props & props, uint16_t id)) allocator)(controllerProps, controllerId++);
    lastPluginControllerInstance = plugin.instance;
    logDebug("plugin interface loaded: %s\n", path);

    controllers.push_back(plugin);
}

#endif
