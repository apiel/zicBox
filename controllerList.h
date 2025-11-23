/** Description:
This file serves as the central directory or registry for managing specialized software components known as "Controllers." It acts like a standardized system for organizing and retrieving functional modules, often used in plugin architectures.

### How It Works

1.  **Defining the Blueprint:** The file establishes a standard blueprint named `Controller`. Every software module registered in the system must conform to this blueprint. It ensures that each module stores two vital pieces of information:
    *   A simple text label (its unique name).
    *   The actual running program component (the ‘instance’) that handles its specific control logic, defined by an imported `ControllerInterface`.

2.  **Creating the Registry:** A central, dynamic list, named `controllers`, is created. This list acts as the master phone book, storing all available controller blueprints the system knows about.

3.  **The Lookup Function:** The primary utility of the file is the `getController` function. When another part of the program needs a specific controller—for example, one named "Temperature_Monitor"—it calls this function using the name. The function efficiently searches the master list of controllers. If a name match is found, it delivers the corresponding active program component for use; otherwise, it signals that the requested component is not available.

In summary, this header file provides the structure and searching mechanism necessary to allow a large program to efficiently manage, organize, and access various specialized, plug-and-play functional units by name.

sha: f732d04ff02a9ff630f24faaa75a8732346a650538cea1561bc199f35e5bd191 
*/
#pragma once

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
