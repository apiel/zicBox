#ifndef _CONTROLLER_LIST_H_
#define _CONTROLLER_LIST_H_

#include <vector>
#include <string.h>

#include "plugins/controllers/controllerInterface.h"

struct Controller {
    char name[64];
    ControllerInterface* instance;
};
std::vector<Controller> controllers;

ControllerInterface* getController(const char* name)
{
    for (auto& controller : controllers) {
        if (strcmp(controller.name, name) == 0) {
            return controller.instance;
        }
    }
    return NULL;
}

#endif
